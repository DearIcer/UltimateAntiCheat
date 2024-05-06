//By AlSch092 @github
#pragma once
#include <winsock2.h>
#include <Iphlpapi.h>
#include <list>
#include <stdint.h>
#include <string>

#include "../Common/Error.hpp"
#include "../Logger.hpp"
#include "Packets/Packets.hpp"
#include "../Process/Process.hpp"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "iphlpapi.lib")

#define DEFAULT_RECV_LENGTH 512
#define MINIMUM_PACKET_SIZE 4

using namespace std;

/*
Class NetClient - Client-side of networking portion
*/
class NetClient
{
public:

	NetClient()
	{
		HandshakeCompleted = false;
		Initialized = false;
	}

	NetClient(const char* serverEndpoint, uint16_t port)
	{
		Ip = serverEndpoint;
		Port = port;

		HandshakeCompleted = false;
		Initialized = false;
	}

	static void ProcessRequests(LPVOID Param); //calls recv in a loop to handle requests, and if this routine is not running the program should be exited

	Error Initialize(string ip, uint16_t port); //connects, sends CS_HELLO, verifies the response of a version number from server
	Error EndConnection(int reason); //sends CS_GOODBYE and disconnects the socket
	Error SendData(PacketWriter* outPacket); //all data sent to the server after CS_HELLO should go through this

	Error QueryMemory(PacketWriter* p); //query specific memory address, send its bytes values back to server

	static string GetHostname();
	string GetMACAddress();
	string GetHardwareID();

	uint64_t MakeHashFromServerResponse(PacketWriter* p);
	Error HandleInboundPacket(PacketWriter* p);

	bool ExecutePacketPayload(PacketWriter* p); //unpacks receive packet which contains a secret key + payload

	bool HandshakeCompleted = false;
	bool Initialized = false;

	SOCKET GetClientSocket() { return this->Socket; }
	string GetConnectedIP() { return this->Ip; }
	uint16_t GetConnectedPort() { return this->Port; }
	list<uint64_t> GetResponseHashList() { return this->HeartbeatHashes; }

private:

	SOCKET Socket = SOCKET_ERROR;

	bool Connected = false;

	string Ip;
	uint16_t Port = 0;

	unsigned int ConnectedDuration = 0;
	unsigned long long ConnectedAt = 0; //unix timestamp

	string Hostname;
	string HardwareID;
	string MACAddress;

	Error LastError = Error::OK;

	HANDLE RecvLoopThread = NULL;
	DWORD recvThreadId = 0;

	list<uint64_t> HeartbeatHashes; //each next reply should be built using the hash of the last response, similar to a blockchain . if this goes out of sync at any point, server d/cs client
};

