#ifndef LIBTORRENT_DOWNLOAD_H
#define LIBTORRENT_DOWNLOAD_H

#include <torrent/common.h>
#include <torrent/entry.h>
#include <torrent/peer.h>

#include <list>
#include <vector>
#include <sigc++/signal.h>

namespace torrent {

typedef std::list<Peer> PList;

// Not a real struct.
struct DownloadWrapper;

class Download {
public:
  typedef std::vector<uint16_t> SeenVector;

  enum {
    NUMWANT_DISABLED = -1
  };

  Download(DownloadWrapper* d = NULL) : m_ptr(d) {}

  // Not active atm
  void                 open();
  void                 close();

  void                 start();
  void                 stop();

  // Does not check if the download has been removed.
  bool                 is_valid()  { return m_ptr; }

  bool                 is_open();
  bool                 is_active();
  bool                 is_tracker_busy();

  // Returns "" if the object is not valid.
  std::string          get_name();
  std::string          get_hash();

  // Only set the root directory while the torrent is closed.
  std::string          get_root_dir();
  void                 set_root_dir(const std::string& dir);

  // Bytes uploaded this session.
  uint64_t             get_bytes_up();
  // Bytes downloaded this session.
  uint64_t             get_bytes_down();
  // Bytes completed.
  uint64_t             get_bytes_done();
  // Size of the torrent.
  uint64_t             get_bytes_total();

  uint32_t             get_chunks_size();
  uint32_t             get_chunks_done();
  uint32_t             get_chunks_total();

  // Bytes per second.
  uint32_t             get_rate_up();
  uint32_t             get_rate_down();
  
  const unsigned char* get_bitfield_data();
  uint32_t             get_bitfield_size();

  uint32_t             get_peers_min();
  uint32_t             get_peers_max();
  uint32_t             get_peers_connected();
  uint32_t             get_peers_not_connected();

  uint32_t             get_uploads_max();
  
  uint64_t             get_tracker_timeout();
  int16_t              get_tracker_numwant();

  void                 set_peers_min(uint32_t v);
  void                 set_peers_max(uint32_t v);

  void                 set_uploads_max(uint32_t v);

  void                 set_tracker_timeout(uint64_t v);
  void                 set_tracker_numwant(int16_t n);

  Entry                get_entry(uint32_t i);
  uint32_t             get_entry_size();

  const SeenVector&    get_seen();

  // Call this when you want the modifications of the download priorities
  // in the entries to take effect.
  void                 update_priorities();

  void                 peer_list(PList& pList);
  Peer                 peer_find(const std::string& id);

  // Signals:

  typedef sigc::signal0<void>              SignalDownloadDone;
  typedef sigc::signal1<void, Peer>        SignalPeerConnected;
  typedef sigc::signal1<void, Peer>        SignalPeerDisconnected;
  typedef sigc::signal0<void>              SignalTrackerSucceded;
  typedef sigc::signal1<void, std::string> SignalTrackerFailed;

  typedef sigc::slot0<void>                SlotDownloadDone;
  typedef sigc::slot1<void, Peer>          SlotPeerConnected;
  typedef sigc::slot1<void, Peer>          SlotPeerDisconnected;
  typedef sigc::slot0<void>                SlotTrackerSucceded;
  typedef sigc::slot1<void, std::string>   SlotTrackerFailed;

  SignalDownloadDone::iterator    signal_download_done(SlotDownloadDone s);
  SignalPeerConnected::iterator   signal_peer_connected(SlotPeerConnected s);
  SignalPeerConnected::iterator   signal_peer_disconnected(SlotPeerConnected s);
  SignalTrackerSucceded::iterator signal_tracker_succeded(SlotTrackerSucceded s);
  SignalTrackerFailed::iterator   signal_tracker_failed(SlotTrackerFailed s);

private:
  DownloadWrapper*      m_ptr;
};

}

#endif
