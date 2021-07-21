#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if(!seg.header().syn && !_syn) return;
    _eof = false;
    if(!_syn){
        _syn = true;
        _isn = seg.header().seqno;
        if(seg.header().fin){
            _fin = true;
            _eof = true;
        }
        _reassembler.push_substring(seg.payload().copy(), 0, _eof);
        return;
    }
    if(_syn && seg.header().fin){
        _fin = true;
        _eof = true;
    }
    uint64_t checkpoint = stream_out().bytes_written();
    uint64_t abs_seqno = unwrap(seg.header().seqno, _isn, checkpoint);
    uint64_t stream_index = abs_seqno - 1;
    _reassembler.push_substring(seg.payload().copy(), stream_index, _eof);

}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(!_syn) return std::nullopt;
    uint64_t abs_seqno = stream_out().bytes_written();
    if (_fin && stream_out().input_ended()) {
        abs_seqno++;
    }
    return optional<WrappingInt32>(wrap(abs_seqno + 1, _isn));
 }

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
