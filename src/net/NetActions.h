/*=============================================================================
XMOTO

This file is part of XMOTO.

XMOTO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#ifndef __NETACTIONS_H__
#define __NETACTIONS_H__

#include "../include/xm_SDL_net.h"
#include "../xmscene/BasicSceneStructs.h"
#include "BasicStructures.h"
#include <string>
#include <vector>

#define XM_NET_PROTOCOL_VERSION 6
/*
DELTA 1->2:
clientInfos : add xmversion string
DELTA 2->3:
add NA_udpBindValidation
DELTA 3->4:
add chatMessagePP (private/public)
DELTA 4->5
add slaveClientsPoints
DELTA 5->6
add pings
*/

#define NETACTION_MAX_PACKET_SIZE 1024 * 8 // bytes
#define NETACTION_MAX_SUBSRC 4 // maximum 4 players by client
#define XM_NET_MAX_EVENTS_SHOT_SIZE 1024 * 8

class NetClient;
class ServerThread;
class DBuffer;

enum NetActionType {
  TNA_clientInfos,
  TNA_udpBindQuery,
  TNA_udpBind,
  TNA_udpBindValidation,
  TNA_chatMessage,
  TNA_chatMessagePP,
  TNA_serverError,
  TNA_frame,
  TNA_changeName,
  TNA_clientsNumber,
  TNA_clientsNumberQuery,
  TNA_playingLevel,
  TNA_changeClients,
  TNA_slaveClientsPoints,
  TNA_playerControl,
  TNA_clientMode,
  TNA_prepareToPlay,
  TNA_prepareToGo,
  TNA_killAlert,
  TNA_gameEvents,
  TNA_srvCmd,
  TNA_srvCmdAsw,
  TNA_ping
};

struct NetInfosClient {
  int NetId;
  std::string Name;
};

struct NetPointsClient {
  int NetId;
  int Points;
};

struct NetActionU;

class NetAction {
public:
  NetAction(bool i_forceTcp);
  virtual ~NetAction();
  virtual std::string actionKey() = 0;
  virtual NetActionType actionType() = 0;

  virtual void send(TCPsocket *i_tcpsd,
                    UDPsocket *i_udpsd,
                    UDPpacket *i_sendPacket,
                    IPaddress *i_udpRemoteIP);
  void setSource(int i_src, int i_subsrc);

  int getSource() const;
  int getSubSource() const;

  static void getNetAction(NetActionU *o_netAction,
                           void *data,
                           unsigned int len);

  static void logStats();

  /* stats */
  static unsigned int m_biggestTCPPacketSent;
  static unsigned int m_biggestUDPPacketSent;
  static unsigned int m_nbTCPPacketsSent;
  static unsigned int m_nbUDPPacketsSent;
  static unsigned int m_TCPPacketsSizeSent;
  static unsigned int m_UDPPacketsSizeSent;
  /* ***** */

protected:
  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP,
            const void *subPacketData,
            int subPacketLen);

  int m_source; // source client
  int m_subsource; // a client can have several players : 0, 1, 2 or 3
  // the client sending a packet :
  // => (-1, [0,1,2,3])
  // the client receiving a packet from an other client
  // => (x, [0,1,2,3])
  // the client receiving a packet from the server
  // => (-1, 0)
  // the serveur sending a packet to a client
  // => (-1, 0)
  // the server transfering a packet from a client x to others clients
  // => (x, [0,1,2,3])

  static std::string getLine(void *data,
                             unsigned int len,
                             unsigned int *o_local_offset);
  static std::string getLineCheckUTF8(void *data,
                                      unsigned int len,
                                      unsigned int *o_local_offset);

private:
  static char m_buffer[NETACTION_MAX_PACKET_SIZE];

  bool m_forceTCP; // by default, xmoto try to use UDP when available ; for some
  // actions, TCP can be forced
};

class NA_udpBind : public NetAction {
public:
  NA_udpBind(const std::string &i_key = "");
  NA_udpBind(void *data, unsigned int len);
  virtual ~NA_udpBind();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  std::string key() const;

private:
  std::string m_key;
};

class NA_udpBindQuery : public NetAction {
public:
  NA_udpBindQuery();
  NA_udpBindQuery(void *data, unsigned int len);
  virtual ~NA_udpBindQuery();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

private:
};

class NA_udpBindValidation : public NetAction {
public:
  NA_udpBindValidation();
  NA_udpBindValidation(void *data, unsigned int len);
  virtual ~NA_udpBindValidation();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

private:
};

/*
CLI : >clientInfos (send the udp key)
SRV : >udpBindQuery
CLI : <udpBindQuery ; >udpBind
SRV : <udpBind (ok, i can *receive* udp from client) >udpBindValidation ;
>udpBind
CLI : <udpBindValidation (ok, i can *send* udp to the server)
CLI : <udpBind (ok, i can *receive* udp from the server) ; >udpBindValidation
SRV : <udpBindValidation (ok, i can *send* udp to the client)
*/

class NA_clientInfos : public NetAction {
public:
  NA_clientInfos(int i_protocolVersion = -1,
                 const std::string &i_udpBindKey = "");
  NA_clientInfos(void *data, unsigned int len);
  virtual ~NA_clientInfos();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  int protocolVersion() const;
  std::string udpBindKey() const;
  std::string xmversion() const;

private:
  int m_protocolVersion;
  std::string m_udpBindKey;
  std::string m_xmversion;
};

/* replace chatMessage ; it allows public and private messages */
class NA_chatMessagePP : public NetAction {
public:
  NA_chatMessagePP()
    : NetAction(true) {} // for preallocation
  NA_chatMessagePP(
    const std::string &i_msg,
    const std::string &i_me /* if empty, change nothing */,
    const std::vector<int> &
      i_private_people /* empty vector for public message -- require XM_NET_PROTOCOL_VERSION */);
  NA_chatMessagePP(void *data, unsigned int len);
  virtual ~NA_chatMessagePP();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  std::string getMessage();
  const std::vector<int> &privatePeople() const; // empty for public message

private:
  std::string m_msg;
  std::vector<int> m_private_people;

  void ttransform(const std::string &i_me);
};

class NA_chatMessage : public NetAction {
public:
  NA_chatMessage(const std::string &i_msg = "", const std::string &i_me = "");
  NA_chatMessage(void *data, unsigned int len);
  virtual ~NA_chatMessage();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  std::string getMessage();

private:
  std::string m_msg;

  void ttransform(const std::string &i_me);
};

class NA_serverError : public NetAction {
public:
  NA_serverError(const std::string &i_msg = "");
  NA_serverError(void *data, unsigned int len);
  virtual ~NA_serverError();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  std::string getMessage();

private:
  std::string m_msg;
};

class NA_frame : public NetAction {
public:
  NA_frame(SerializedBikeState *i_state = NULL);
  NA_frame(void *data, unsigned int len);
  virtual ~NA_frame();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  SerializedBikeState *getState();

private:
  SerializedBikeState m_state;
};

class NA_changeName : public NetAction {
public:
  NA_changeName(const std::string &i_name = "");
  NA_changeName(void *data, unsigned int len);
  virtual ~NA_changeName();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  std::string getName();

private:
  std::string m_name;
};

class NA_clientsNumber : public NetAction {
public:
  NA_clientsNumber(int i_ntcp = 0,
                   int i_nudp = 0,
                   int i_nghosts = 0,
                   int i_nslaves = 0);
  NA_clientsNumber(void *data, unsigned int len);
  virtual ~NA_clientsNumber();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  int getNumberTCP() const;
  int getNumberUDP() const;
  int getNumberGhosts() const;
  int getNumberSlaves() const;

private:
  int m_ntcp;
  int m_nudp;
  int m_nghosts;
  int m_nslaves;
};

// query the number of clients for the munin plugin
class NA_clientsNumberQuery : public NetAction {
public:
  NA_clientsNumberQuery();
  NA_clientsNumberQuery(void *data, unsigned int len);
  virtual ~NA_clientsNumberQuery();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);
};

class NA_playingLevel : public NetAction {
public:
  NA_playingLevel(const std::string &i_levelId = "");
  NA_playingLevel(void *data, unsigned int len);
  virtual ~NA_playingLevel();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  std::string getLevelId();

private:
  std::string m_levelId;
};

class NA_changeClients : public NetAction {
public:
  NA_changeClients();
  NA_changeClients(void *data, unsigned int len);
  virtual ~NA_changeClients();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void add(NetInfosClient *i_infoClient);
  void remove(NetInfosClient *i_infoClient);
  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  const std::vector<NetInfosClient> &getAddedInfosClients() const;
  const std::vector<NetInfosClient> &getRemovedInfosClients() const;

private:
  std::vector<NetInfosClient> m_netAddedInfosClients;
  std::vector<NetInfosClient> m_netRemovedInfosClients;
};

class NA_playerControl : public NetAction {
public:
  NA_playerControl(PlayerControl i_control, float i_value);
  NA_playerControl(PlayerControl i_control = PC_CHANGEDIR, bool i_value = true);
  NA_playerControl(void *data, unsigned int len);
  virtual ~NA_playerControl();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  PlayerControl getType();
  float getFloatValue();
  bool getBoolValue();

private:
  PlayerControl m_control;
  float m_value;
};

class NA_clientMode : public NetAction {
public:
  NA_clientMode(NetClientMode i_mode = NETCLIENT_GHOST_MODE);
  NA_clientMode(void *data, unsigned int len);
  virtual ~NA_clientMode();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  NetClientMode mode() const;

private:
  NetClientMode m_mode;
};

class NA_prepareToPlay : public NetAction {
public:
  NA_prepareToPlay()
    : NetAction(false) {}
  NA_prepareToPlay(const std::string &i_id_level, std::vector<int> &i_players);
  NA_prepareToPlay(void *data, unsigned int len);
  virtual ~NA_prepareToPlay();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  std::string idLevel() const;
  const std::vector<int> &players();

private:
  std::string m_id_level;
  std::vector<int> m_players;
};

class NA_killAlert : public NetAction {
public:
  NA_killAlert(int i_time = 0);
  NA_killAlert(void *data, unsigned int len);
  virtual ~NA_killAlert();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  int time() const;

private:
  int m_time;
};

class NA_prepareToGo : public NetAction {
public:
  NA_prepareToGo(int i_time = 0);
  NA_prepareToGo(void *data, unsigned int len);
  virtual ~NA_prepareToGo();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  int time() const;

private:
  int m_time;
};

class NA_gameEvents : public NetAction {
public:
  NA_gameEvents(DBuffer *i_buffer = NULL);
  NA_gameEvents(void *data, unsigned int len);
  virtual ~NA_gameEvents();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  char *buffer();
  int bufferSize();

private:
  char m_buffer[XM_NET_MAX_EVENTS_SHOT_SIZE];
  int m_bufferLength;
};

class NA_srvCmd : public NetAction {
public:
  NA_srvCmd(const std::string &i_cmd = "");
  NA_srvCmd(void *data, unsigned int len);
  virtual ~NA_srvCmd();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  std::string getCommand();

private:
  std::string m_cmd;
};

class NA_srvCmdAsw : public NetAction {
public:
  NA_srvCmdAsw(const std::string &i_answer = "");
  NA_srvCmdAsw(void *data, unsigned int len);
  virtual ~NA_srvCmdAsw();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  std::string getAnswer();

private:
  std::string m_answer;
};

class NA_slaveClientsPoints : public NetAction {
public:
  NA_slaveClientsPoints();
  NA_slaveClientsPoints(void *data, unsigned int len);
  virtual ~NA_slaveClientsPoints();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  const std::vector<NetPointsClient> &getPointsClients() const;
  void add(NetPointsClient *i_npc);

private:
  std::vector<NetPointsClient> m_netPointsClients;
};

class NA_ping : public NetAction {
public:
  NA_ping(NA_ping *i_ping = NULL); // NULL if that's a ping, the ping for a pong
  NA_ping(void *data, unsigned int len);
  virtual ~NA_ping();
  std::string actionKey() { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket *i_tcpsd,
            UDPsocket *i_udpsd,
            UDPpacket *i_sendPacket,
            IPaddress *i_udpRemoteIP);

  int id() const;
  bool isPong() const;

private:
  static int m_currentId; // shared id

  int m_id; // unique identifier
  bool m_isPong; // is it an answer of a ping ?
};

/* structure to avoid allocation of the NetAction while netaction are read and
 * manage one by one */
struct NetActionU {
public:
  NetActionU() {}

  NetAction *master;
  NA_clientInfos clientInfos;
  NA_udpBindQuery udpBindQuery;
  NA_udpBind udpBind;
  NA_udpBindValidation udpBindValidation;
  NA_chatMessage chatMessage;
  NA_chatMessagePP chatMessagePP;
  NA_serverError serverError;
  NA_frame frame;
  NA_changeName changeName;
  NA_clientsNumber clientsNumber;
  NA_clientsNumberQuery clientsNumberQuery;
  NA_playingLevel playingLevel;
  NA_changeClients changeClients;
  NA_slaveClientsPoints slaveClientsPoints;
  NA_playerControl playerControl;
  NA_clientMode clientMode;
  NA_prepareToPlay prepareToPlay;
  NA_prepareToGo prepareToGo;
  NA_killAlert killAlert;
  NA_gameEvents gameEvents;
  NA_srvCmd srvCmd;
  NA_srvCmdAsw srvCmdAsw;
  NA_ping ping;
};

#endif
