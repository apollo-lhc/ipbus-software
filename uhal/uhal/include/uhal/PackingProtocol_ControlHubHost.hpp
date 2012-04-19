/**
	@file
	@author Andrew W. Rose
	@date 2012
*/

#ifndef _uhal_PackingProtocol_ControlHubHost_hpp_
#define _uhal_PackingProtocol_ControlHubHost_hpp_

#include "uhal/ProtocolInterfaces.hpp"
#include "boost/system/error_code.hpp"
#include "uhal/AsioAccumulatedPacket.hpp"

namespace uhal
{

	template< eIPbusProtocolVersion IPbusProtocolVersion >
	class ControlHubHostPackingProtocol : public PackingProtocol
	{
		public:
			/**
				Constructor
			*/
			ControlHubHostPackingProtocol ( const uint32_t& aMaxPacketLength );

			/**
				Destructor
			*/
			virtual ~ControlHubHostPackingProtocol();

			/**
				A function which adds an IPbusPacketInfo to the queue of pending IPbus transactions
				@param aIPbusPacketInfo an IPbusPacketInfo to be added to the queue of pending IPbus transactions
				@param aId the identifier of the target device (IPbus client instance), since the control-hub host handles more than one device
			*/
			void pack ( IPbusPacketInfo& aIPbusPacketInfo , const uint32_t& aId = 0 );

			/**
				A function called immediately prior to a call to the Transport Protocol's Dispatch() function. Used for finalizing the queue for dispatch.
			*/
			void PreDispatch();

			/**
				A function called immediately after to a call to the Transport Protocol's Dispatch() function. Used for testing that the replys are as expected.
			*/
			void PostDispatch();


			void ReceiveHandler ( const boost::system::error_code& aErrorCode, std::size_t aReplyLength, std::size_t& aReplyLengthRef , bool& aAwaitingCallBackRef , bool& aErrorRef );

			void GotPreamble ( std::size_t aReplyLength , bool& aError );
			void GotHeader ( std::size_t aReplyLength , bool& aError );
			void GotPayload ( std::size_t aReplyLength , bool& aError );

			inline const tAccumulatedPackets& getAccumulatedPackets();

		private:
			//! The raw stream packets which will be sent by the transport protocol
			tAccumulatedPackets mAccumulatedPackets;

			/**
				The maximum allowed size of a chunk
				@todo Now that the chunking is templated, could this be moved into the IPbusPacketInfo?
			*/
			uint32_t mMaxPacketLength;

			/** A register from which to issue byte order transactions
				@note We need to pad out the packets to compensate for the bug in the firmware
				If we didn't then the mByteOrderTransaction could be a single 32bit uint (as it was previously)
			*/
			uint32_t mByteOrderTransaction[8];

			//! A register into which to write the transaction reply header
			uint32_t mDeviceIDheader;

			//! A register to receive the reply bytecount header
			uint32_t mReplyByteCount;

			//! A register into which to write the transaction reply header
			uint32_t mTransactionId;

			/**
				Each time a transaction is added for a particular device,
				make a note where the last entry was so that subsequent
				entries are not stored before it!
			*/
			std::map< uint32_t , uint32_t > mLastInstruction;


			/**
				A deque to hold the counter fields
				Stored as a local copy so that if user sets the value by temporary variable, the data will be valid when sent
			*/
			std::deque<uint32_t> mCounters;

			//! A map of device IDs to reply packets.
			typedef std::deque< std::pair< uint32_t , IPbusPacketInfo::tChunks* > > tChunkList;
			typedef std::map< uint32_t , tChunkList > tReplyMap;
			tReplyMap mReplyMap;
			tReplyMap::iterator mReplyMapIt;


			uint32_t* mReplyMemory;
			uint32_t* mTransactionHeader;
			uint32_t mDeviceIndex;
			uint32_t mTemp1;
			uint32_t mTemp2;

			enum { ExpectPreamble , ExpectHeader , ExpectPayload } mExpectedReplyType ;

			//! Instance of the deque storing the data to be sent and associated packet header information
			tIPbusPacketInfoStorage mPacketInfo;


	};

}

#include "TemplateDefinitions/PackingProtocol_ControlHubHost.hxx"


#endif
