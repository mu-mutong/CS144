#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _buffer(capacity, '\0'), _bytesIndex(capacity, false) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (index > _head + _output.remaining_capacity())
        return;
    if (index + data.size() < _head) {
        if (eof)
            _eof = true;
    } else {
        if (index + data.size() <= _head + _output.remaining_capacity()) {
            if (eof)
                _eof = true;
        }
        for (size_t i = index; i < _head + _output.remaining_capacity() && i < index + data.size(); i++) {
            if (i >= _head && !_bytesIndex[i - _head]) {
                _buffer[i - _head] = data[i - index];
                _bytesIndex[i - _head] = true;
                _unassembled_bytes++;
            }
        }
        std::string str = "";
        while (_bytesIndex.front()) {
            str += _buffer.front();
            _buffer.pop_front();
            _buffer.push_back('\0');
            _bytesIndex.pop_front();
            _bytesIndex.push_back(false);
        }
        if (str.size() > 0) {
            _unassembled_bytes -= str.size();
            _head += str.size();
            _output.write(str);
        }
    }
    if (_eof && empty()) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
