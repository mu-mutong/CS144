#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

// size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

// size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

// size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

// size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

// void TCPConnection::segment_received(const TCPSegment &seg) {
//     if (!_active)
//         return;
//     _time_since_last_segment_received = 0;

//     // data segments with acceptable ACKs should be ignored in SYN_SENT
//     if (in_syn_sent() && seg.header().ack && seg.payload().size() > 0) {
//         return;
//     }
//     bool send_empty = false;
//     if (_sender.next_seqno_absolute() > 0 && seg.header().ack) {
//         // unacceptable ACKs should produced a segment that existed
//         if (!_sender.ack_received(seg.header().ackno, seg.header().win)) {
//             send_empty = true;
//         }
//     }

//     bool recv_flag = _receiver.segment_received(seg);
//     if (!recv_flag) {
//         send_empty = true;
//     }

//     if (seg.header().syn && _sender.next_seqno_absolute() == 0) {
//         connect();
//         return;
//     }

//     if (seg.header().rst) {
//         // RST segments without ACKs should be ignored in SYN_SENT
//         if (in_syn_sent() && !seg.header().ack) {
//             return;
//         }
//         unclean_shutdown(false);
//         return;
//     }

//     if (seg.length_in_sequence_space() > 0) {
//         send_empty = true;
//     }

//     if (send_empty) {
//         if (_receiver.ackno().has_value() && _sender.segments_out().empty()) {
//             _sender.send_empty_segment();
//         }
//     }
//     push_segments_out();
// }

// bool TCPConnection::active() const { return _active; }

// size_t TCPConnection::write(const string &data) {
//     size_t ret = _sender.stream_in().write(data);
//     push_segments_out();
//     return ret;
// }

// //! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
// void TCPConnection::tick(const size_t ms_since_last_tick) {
//     if (!_active)
//         return;
//     _time_since_last_segment_received += ms_since_last_tick;
//     _sender.tick(ms_since_last_tick);
//     if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
//         unclean_shutdown(true);
//     }
//     push_segments_out();
// }

// void TCPConnection::end_input_stream() {
//     _sender.stream_in().end_input();
//     push_segments_out();
// }

// void TCPConnection::connect() {
//     // when connect, must active send a SYN
//     push_segments_out(true);
// }

// TCPConnection::~TCPConnection() {
//     try {
//         if (active()) {
//             // Your code here: need to send a RST segment to the peer
//             cerr << "Warning: Unclean shutdown of TCPConnection\n";
//             unclean_shutdown(true);
//         }
//     } catch (const exception &e) {
//         std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
//     }
// }

// bool TCPConnection::push_segments_out(bool send_syn) {
//     // default not send syn before recv a SYN
//     _sender.fill_window(send_syn || in_syn_recv());
//     TCPSegment seg;
//     while (!_sender.segments_out().empty()) {
//         seg = _sender.segments_out().front();
//         _sender.segments_out().pop();
//         if (_receiver.ackno().has_value()) {
//             seg.header().ack = true;
//             seg.header().ackno = _receiver.ackno().value();
//             seg.header().win = _receiver.window_size();
//         }
//         if (_need_send_rst) {
//             _need_send_rst = false;
//             seg.header().rst = true;
//         }
//         _segments_out.push(seg);
//     }
//     clean_shutdown();
//     return true;
// }

// void TCPConnection::unclean_shutdown(bool send_rst) {
//     _receiver.stream_out().set_error();
//     _sender.stream_in().set_error();
//     _active = false;
//     if (send_rst) {
//         _need_send_rst = true;
//         if (_sender.segments_out().empty()) {
//             _sender.send_empty_segment();
//         }
//         push_segments_out();
//     }
// }

// bool TCPConnection::clean_shutdown() {
//     if (_receiver.stream_out().input_ended() && !(_sender.stream_in().eof())) {
//         _linger_after_streams_finish = false;
//     }
//     if (_sender.stream_in().eof() && _sender.bytes_in_flight() == 0 && _receiver.stream_out().input_ended()) {
//         if (!_linger_after_streams_finish || time_since_last_segment_received() >= 10 * _cfg.rt_timeout) {
//             _active = false;
//         }
//     }
//     return !_active;
// }

// bool TCPConnection::in_listen() { return !_receiver.ackno().has_value() && _sender.next_seqno_absolute() == 0; }

// bool TCPConnection::in_syn_recv() { return _receiver.ackno().has_value() && !_receiver.stream_out().input_ended(); }

// bool TCPConnection::in_syn_sent() {
//     return _sender.next_seqno_absolute() > 0 && _sender.bytes_in_flight() == _sender.next_seqno_absolute();
// }

void TCPConnection::send_all_segments() {
    if (!_active)
        return;
    while (!_sender.segments_out().empty()) {
        TCPSegment &seg = _sender.segments_out().front();
        if (_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
        }
        size_t max_win = numeric_limits<uint16_t>().max();
        seg.header().win = min(_receiver.window_size(), max_win);
        _segments_out.push(seg);
        _sender.segments_out().pop();
    }

    if (_sender.stream_in().eof() && _sender.bytes_in_flight() == 0 && _receiver.stream_out().input_ended()) {
        if (_linger_after_streams_finish) {
            _time_wait = true;
        }
    }
}

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _ticks - _last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if (!_syn_sent && !seg.header().syn)
        return;
    _rst = seg.header().rst;
    _last_segment_received = _ticks;
    if (_rst) {
        //_activate = false;
        _receiver.stream_out().set_error();
        _sender.stream_in().set_error();
        _linger_after_streams_finish = false;
        //_time_wait = false;
    }
    _receiver.segment_received(seg);
    _sender.ack_received(seg.header().ackno, seg.header().win);
    _sender.fill_window();
    _syn_sent = true;

    if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof()) {
        // passive close
        _linger_after_streams_finish = false;
        _time_wait = false;
    }
    if (!_receiver.ackno().has_value()) {
        return;  // no need for ack
    }
    if (_sender.segments_out().empty()) {
        // generate an empty segment to ack
        if (_receiver.stream_out().input_ended() && !seg.header().fin) {
            // no need to ack, server closed and seg not fin
        } else if (seg.length_in_sequence_space() == 0) {
            // no need to ack the empty-ack
        } else {
            _sender.send_empty_segment();
        }
    }
    send_all_segments();
}

bool TCPConnection::active() const {
    if (_sender.stream_in().error() && _receiver.stream_out().error())
        return false;
    return !(_sender.stream_in().eof() && _sender.bytes_in_flight() == 0 && _receiver.stream_out().input_ended()) ||
           _time_wait;
}

size_t TCPConnection::write(const string &data) {
    size_t _write_length = _sender.stream_in().write(data);
    _sender.fill_window();
    send_all_segments();
    return _write_length;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _ticks += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);

    if (_sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS) {
        // RST
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _linger_after_streams_finish = false;
        while (!_sender.segments_out().empty()) {
            // pop all segments
            _sender.segments_out().pop();
        }
        _sender.send_empty_segment();
        TCPSegment &seg = _sender.segments_out().front();
        seg.header().rst = true;
    }
    send_all_segments();
    if (_time_wait && _ticks >= _last_segment_received + _cfg.rt_timeout * 10) {
        if (_sender.stream_in().eof() && _sender.bytes_in_flight() == 0 && _receiver.stream_out().input_ended()) {
            _time_wait = false;
            _active = false;
        }
        // closed
    }
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    send_all_segments();
}

void TCPConnection::connect() {
    if (!_syn_sent) {
        _sender.fill_window();
        TCPSegment &seg = _sender.segments_out().front();
        size_t max_win = numeric_limits<uint16_t>().max();
        seg.header().win = min(_receiver.window_size(), max_win);
        _segments_out.push(_sender.segments_out().front());
        _sender.segments_out().pop();
        _syn_sent = true;
    }
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            _sender.stream_in().set_error();
            _receiver.stream_out().set_error();
            _linger_after_streams_finish = false;
            _time_wait = false;
            while (!_sender.segments_out().empty()) {
                // pop all segments
                _sender.segments_out().pop();
            }
            _sender.send_empty_segment();
            TCPSegment &seg = _sender.segments_out().front();
            seg.header().rst = true;
            send_all_segments();
            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
