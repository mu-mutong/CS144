#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _capacity(capacity) {}

size_t ByteStream::write(const string &data) {
    size_t wlen = data.length();
    if (wlen > (_capacity - buffer.size())) {
        wlen = _capacity - buffer.size();
    }
    for (size_t i = 0; i < wlen; i++) {
        buffer.push_back(data[i]);
    }
    totalWrite += wlen;

    return wlen;
}

//! \param[in] len bytes will be copied from the output side of the buffer
std::string ByteStream::peek_output(const size_t len) const {
    size_t plen = len;
    if (plen > buffer.size()) {
        plen = buffer.size();
    }
    /*string reads(plen, 0);

    std::vector<char>::const_iterator it = buffer.begin();
    for (size_t i = 0; i < plen; i++) {
        reads[i] = *it;
        it++;
    }
    return reads;*/

    return string().assign(buffer.begin(), buffer.begin() + plen);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t plen = len;
    if (plen > buffer.size()) {
        plen = buffer.size();
    }
    totalRead += plen;
    buffer.erase(buffer.begin(), buffer.begin() + plen);
    /*std::vector<char>::const_iterator it = buffer.begin();
    for (size_t i = 0; i < plen; i++) {
        buffer.erase(it);
        // it++;
    }*/
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string result = peek_output(len);
    pop_output(len);
    return result;
}

void ByteStream::end_input() { end = true; }

bool ByteStream::input_ended() const { return end; }

size_t ByteStream::buffer_size() const { return buffer.size(); }

bool ByteStream::buffer_empty() const { return buffer.size() == 0; }

bool ByteStream::eof() const { return buffer_empty() && input_ended(); }

size_t ByteStream::bytes_written() const { return totalWrite; }

size_t ByteStream::bytes_read() const { return totalRead; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer.size(); }
