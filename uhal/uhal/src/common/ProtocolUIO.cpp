#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <uhal/Node.hpp>
#include <uhal/NodeTreeBuilder.hpp>
#include <pugixml.hpp>

#include <uhal/ProtocolUIO.hpp>



using namespace uioaxi;
using namespace boost::filesystem;

namespace uhal {

UIO::UIO (
      const std::string& aId, const URI& aUri,
      const boost::posix_time::time_duration&aTimeoutPeriod
    ) :
      ClientInterface(aId,aUri,aTimeoutPeriod)
{
  for (int i=0; i<uioaxi::DEVICES_MAX ; i++){ fd[i]=-1; hw[i]=NULL; }
  // Get the filename of the address table from the connection file. Then read it through the NodeTreeBuilder
  // The NodeTreeBuilder should be able to just use the existing node tree rather than rebuild a new one
  NodeTreeBuilder & mynodetreebuilder = NodeTreeBuilder::getInstance();
  //boost::shared_ptr< Node > lNode ( mynodetreebuilder.getNodeTree ( tabfname , boost::filesystem::current_path() / "." ) );
  Node* lNode = ( mynodetreebuilder.getNodeTree ( std::string("file://")+aUri.mHostname , boost::filesystem::current_path() / "." ) );
  // Getting the IDs for only first layer nodes (nodes that contain device labels). matching names that doesn't contain a "."
  std::vector< std::string > top_node_Ids = lNode->getNodes("^[^.]+$");
  // For each device label, search for its matching device
  for (std::vector<std::string>::iterator nodeId = top_node_Ids.begin(); nodeId != top_node_Ids.end(); ++nodeId) {
    // device number is the number read from the most significant 8 bits of the address
    // size should be read from /sys/class/uio*/maps/map0/size
    int devnum=-1, size=0;
    char label[128]="", uioname[128]="", sizechar[128]="", addrchar[128]=""; 
    uint32_t address1=0, address2=0;
    // get the device number out from the node
    devnum = decodeAddress(lNode->getNode(*nodeId).getAddress()).device;
    // search through the file system to see if there is a uio that matches the name
    std::string uiopath = "/sys/class/uio/";
    std::string dvtpath = "/proc/device-tree/amba_pl/";
    FILE *labelfile=0; 
    FILE *addrfile=0;
    FILE *sizefile=0; 
    // traverse through the device-tree
    for (directory_iterator x(dvtpath); x!=directory_iterator(); ++x){
      if (!is_directory(x->path())) {
        continue;
      }
      if (!exists(x->path()/"label")) {
        continue;
      }
      labelfile = fopen((x->path().native()+"/label").c_str(),"r");
      fgets(label,128,labelfile); fclose(labelfile);
      if(!strcmp(label, (*nodeId).c_str())){
        //get address1 from the file name
        int namesize=x->path().filename().native().size();
        if(namesize<10) {
          fprintf(stderr, "directory name %s has incorrect format\n", x->path().filename().native().c_str());
          continue; //expect the name to be in x@xxxxxxxx format for example myReg@0x41200000
        }
        address1 = std::strtoul( x->path().filename().native().substr(namesize-8,8).c_str() , 0, 16);
        break;
      }
    }
    if(address1==0) fprintf(stderr, "Error: cannot find a device that matches label %s, device not opened!\n", (*nodeId).c_str());

    // Traverse through the /sys/class/uio directory
    for (directory_iterator x(uiopath); x!=directory_iterator(); ++x){
      if (!is_directory(x->path())) {
        continue;
      }
      if (!exists(x->path()/"maps/map0/addr")) {
        continue;
      }
      if (!exists(x->path()/"maps/map0/size")) {
        continue;
      }
      addrfile = fopen((x->path()/"maps/map0/addr").native().c_str(),"r");
      fgets(addrchar,128,addrfile); fclose(addrfile);
      address2 = std::strtoul( addrchar, 0, 16);
      if (address1 == address2){
        sizefile = fopen((x->path().native()+"/maps/map0/size").c_str(),"r");
        fgets(sizechar,128,sizefile); fclose(sizefile);
        //the size was in number of bytes, convert into number of uint32
        size=std::strtoul( sizechar, 0, 16)/4;  
        strcpy(uioname,x->path().filename().native().c_str());
        break;
      }
    }
    if (size==0) {
      fprintf(stderr, "Error: Trouble loading device %s, cannot find device or size zero\n", (*nodeId).c_str());
    }
    //save the mapping
    addrs[devnum]=address1;
    strcpy(uionames[devnum],uioname);
    openDevice(devnum, size, uioname);
  }
}

UIO::~UIO () {
  fprintf(stderr, "UIO: destructor\n");
}

void
UIO::openDevice(int i, uint32_t size, const char *name) {
  if (i<0||i>=DEVICES_MAX) return;
  const char *prefix = "/dev";
  size_t devpath_cap = strlen(prefix)+1+strlen(name)+1;
  char *devpath = (char*)malloc(devpath_cap);
  snprintf(devpath,devpath_cap, "%s/%s", prefix, name);
  fd[i] = open(devpath, O_RDWR|O_SYNC);
  if (-1==fd[i]) {
    fprintf(stderr, "Failed to open %s: %s\n", devpath, strerror(errno));
    goto end;
  }
  hw[i] = (uint32_t*)mmap( NULL, size*sizeof(uint32_t),
                           PROT_READ|PROT_WRITE, MAP_SHARED,
                           fd[i], 0x0);
  if (hw[i]==MAP_FAILED) {
    fprintf(stderr, "Failed to map %s: %s\n", devpath, strerror(errno));
    hw[i]=NULL;
    goto end;
  }
  fprintf(stderr, "Mapped %s as device number 0x%.*x size 0x%x\n", devpath,
      DEVNUMPRLEN,i, size);
  end:
  free(devpath);
}

int
UIO::checkDevice (int i) {
  if (!hw[i]) {
    // Todo: replace with an exception
    fprintf(stderr, "No device with number 0x%.*x\n",
      DEVNUMPRLEN,i);
    return 1;
  }
  return 0;
}

DevAddr
UIO::decodeAddress (uint32_t uaddr) {
  DevAddr da;
  da.device = (uaddr&ADDR_DEV_MASK)>>ADDR_DEV_OFFSET;
  da.word = (uaddr&ADDR_WORD_MASK);
  return da;
}

ValHeader
UIO::implementWrite (const uint32_t& aAddr, const uint32_t& aValue) {
  DevAddr da = decodeAddress(aAddr);
  fprintf(stderr, "UIO: write at uhal addr %08x -> "
    "device 0x%.*x hw addr %08x (byte %08lx), "
    "\n",
    aAddr,DEVNUMPRLEN,da.device,da.word,da.word*sizeof(*hw));
  if (checkDevice(da.device)) return ValWord<uint32_t>();
  uint32_t writeval = aValue;
  hw[da.device][da.word] = writeval;
  fprintf(stderr, "UIO:    wrote value %08x\n", writeval);
  return ValHeader();
}

ValWord<uint32_t>
UIO::implementRead (const uint32_t& aAddr, const uint32_t& aMask) {
  DevAddr da = decodeAddress(aAddr);
  fprintf(stderr, "UIO: read at uhal addr %08x -> "
    "device 0x%.*x hw addr %08x (byte %08lx), "
    "mask %08x\n",
    aAddr,DEVNUMPRLEN,da.device,da.word,da.word*sizeof(*hw),aMask);
  if (checkDevice(da.device)) return ValWord<uint32_t>();
  uint32_t readval = hw[da.device][da.word];
  fprintf(stderr, "UIO:    read value %08x\n", readval);
  ValWord<uint32_t> vw;
  vw=readval;
  valwords.push_back(vw);
  primeDispatch();
  return vw;
}

void
UIO::primeDispatch () {
  // uhal will never call implementDispatch unless told that buffers are in
  // use (even though the buffers are not actually used and are length zero).
  // implementDispatch will only be called once after each checkBufferSpace.
  uint32_t sendcount = 0, replycount = 0, sendavail, replyavail;
  checkBufferSpace ( sendcount, replycount, sendavail, replyavail);
}

void
UIO::implementDispatch (boost::shared_ptr<Buffers> aBuffers) {
  fprintf(stderr, "UIO: Dispatch\n");
  for (unsigned int i=0; i<valwords.size(); i++)
    valwords[i].valid(true);
  valwords.clear();
}

}

