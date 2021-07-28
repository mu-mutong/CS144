#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)


TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
     , _retransmission_timeout(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _next_seqno - _ack_seqno; }

void TCPSender::fill_window() {
    if(!_syn_sent){
        TCPSegment seg;
        seg.header().syn = true;
        seg.header().seqno = wrap(_next_seqno, _isn);
        _segments_out.push(seg);
        _segments_not_ack.push(seg);
        _next_seqno += 1;
        _syn_sent = true;
        if (!_time_running) {
        // open timer
            _time_running = true;
            _timer = 0;
        }    
        return;
    }
    uint64_t remain = _window_size - bytes_in_flight();
    while (remain != 0 && !_fin_sent){
        /* code */
        uint64_t sent_bytes = min(remain, TCPConfig::MAX_PAYLOAD_SIZE);
        string payload = _stream.read(sent_bytes);
        TCPSegment seg;
        seg.header().seqno = wrap(_next_seqno, _isn);
        seg.payload() = move(payload);
        _next_seqno += seg.length_in_sequence_space();
        
        if (_stream.eof() && (seg.length_in_sequence_space() < _window_size)) {
            seg.header().fin = true;
            _next_seqno += 1;
            _fin_sent = true;
        }
        if(seg.length_in_sequence_space() == 0) return;
        _segments_out.push(seg);
        _segments_not_ack.push(seg);
        remain = _window_size - bytes_in_flight();
        if (!_time_running) {
        // open timer
            _time_running = true;
            _timer = 0;
        }    
    }


}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    if(window_size == 0){
        _window_size = 1;
        _windows = true;
    }else{
        _window_size = window_size;
        _windows = false; 
    }
    uint64_t ack = unwrap(ackno,_isn,_ack_seqno);
    if(ack > _next_seqno || ack <= _ack_seqno) return;
    if(ack > _ack_seqno){
        _ack_seqno = ack;
    }
    while (!_segments_not_ack.empty()) {
        TCPSegment seg = _segments_not_ack.front();
        if(seg.length_in_sequence_space() + unwrap(seg.header().seqno, _isn, _ack_seqno) <= _ack_seqno) {
            _segments_not_ack.pop();
        }else {
            break;
        }
    }
    if (!_segments_not_ack.empty()) {
        _time_running = true;
        _timer = 0;
    }
    _retransmission_timeout = _initial_retransmission_timeout;
    _consecutive_retransmission = 0;
 }

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    _timer += ms_since_last_tick;
    if(_timer >= _retransmission_timeout && !_segments_not_ack.empty()){
        _segments_out.push(_segments_not_ack.front());
        _consecutive_retransmission++;
        if(_windows){
            _retransmission_timeout *= 1;
        }else{
            _retransmission_timeout *= 2;
        }
       
        _time_running = true;
        _timer = 0;
    }
    if (_segments_not_ack.empty()) {
        _time_running = false;
        //_timer = 0;
    }
 }

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmission; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}
