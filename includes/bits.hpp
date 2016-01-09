//
//  bits.h
//  ethergen
//
//  Created by Uray Meiviar on 1/3/16.
//  Copyright © 2016 etherlink. All rights reserved.
//

#ifndef bits_h
#define bits_h

#include <stdlib.h>
#include <cstdlib>
#include <stdint.h>
#include <algorithm>
#include <string>
#include <array>

static const char* charTable[] =
{
    "00", "01", "02", "03", "04", "05", "06", "07",
    "08", "09", "0a", "0b", "0c", "0d", "0e", "0f",
    "10", "11", "12", "13", "14", "15", "16", "17",
    "18", "19", "1a", "1b", "1c", "1d", "1e", "1f",
    "20", "21", "22", "23", "24", "25", "26", "27",
    "28", "29", "2a", "2b", "2c", "2d", "2e", "2f",
    "30", "31", "32", "33", "34", "35", "36", "37",
    "38", "39", "3a", "3b", "3c", "3d", "3e", "3f",
    "40", "41", "42", "43", "44", "45", "46", "47",
    "48", "49", "4a", "4b", "4c", "4d", "4e", "4f",
    "50", "51", "52", "53", "54", "55", "56", "57",
    "58", "59", "5a", "5b", "5c", "5d", "5e", "5f",
    "60", "61", "62", "63", "64", "65", "66", "67",
    "68", "69", "6a", "6b", "6c", "6d", "6e", "6f",
    "70", "71", "72", "73", "74", "75", "76", "77",
    "78", "79", "7a", "7b", "7c", "7d", "7e", "7f",
    "80", "81", "82", "83", "84", "85", "86", "87",
    "88", "89", "8a", "8b", "8c", "8d", "8e", "8f",
    "90", "91", "92", "93", "94", "95", "96", "97",
    "98", "99", "9a", "9b", "9c", "9d", "9e", "9f",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af",
    "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7",
    "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf",
    "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7",
    "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf",
    "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
    "d8", "d9", "da", "db", "dc", "dd", "de", "df",
    "e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7",
    "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef",
    "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
    "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff"
};

template <size_t T> class Bits
{
protected:
    std::array<uint8_t,T/8> bytes;
public:
    Bits(){ this->reset(); };
    Bits(const Bits<T>& ref) { this->bytes = ref.bytes; }
    
    uint8_t* ptr(size_t offset = 0) {return ((uint8_t*)this->bytes.data())+offset;};
    uint16_t* ptr16(size_t offset = 0) {return ((uint16_t*)this->bytes.data())+offset;};
    uint32_t* ptr32(size_t offset = 0) {return ((uint32_t*)this->bytes.data())+offset;};
    uint64_t* ptr64(size_t offset = 0) {return ((uint64_t*)this->bytes.data())+offset;};
    
    const uint8_t* ptr(size_t offset = 0) const {return ((const uint8_t*)this->bytes.data())+offset;};
    const uint16_t* ptr16(size_t offset = 0) const {return ((const uint16_t*)this->bytes.data())+offset;};
    const uint32_t* ptr32(size_t offset = 0) const {return ((const uint32_t*)this->bytes.data())+offset;};
    const uint64_t* ptr64(size_t offset = 0) const {return ((const uint64_t*)this->bytes.data())+offset;};
    
    uint16_t value16(size_t index) const { return ((uint16_t*)this->ptr())[index]; };
    uint32_t value32(size_t index) const { return ((uint32_t*)this->ptr())[index]; };
    uint64_t value64(size_t index) const { return ((uint64_t*)this->ptr())[index]; };
    template <typename V> V value(size_t index){
        return ((V*)this->ptr())[index];
    }
    
    void value16(size_t index, uint16_t value) { ((uint16_t*)this->ptr())[index] = value; };
    void value32(size_t index, uint32_t value) { ((uint32_t*)this->ptr())[index] = value; };
    void value64(size_t index, uint64_t value) { ((uint64_t*)this->ptr())[index] = value; };
    template <typename V> void value(size_t index, V value){
        ((V*)this->ptr())[index] = value;
    }
    
    static const size_t bitsSize(){ return T; };
    static const size_t byteSize(){ return T/8; };
    void reset(){ std::fill( std::begin( this->bytes ), std::end( this->bytes ), 0 ); };
    std::string toString() const
    {
        std::string result = "0x";
        for(uint8_t b : this->bytes){
            result += std::string(charTable[b]);
        }
        return result;
    }
    void fromString(const std::string& hexStr)
    {
        std::string input = hexStr;
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);
        if(input.substr(0,2) == "0x")
        {
            input = input.substr(2,input.length()-2);
        }
        if(input.length() % 2 != 0)
        {
            input = "0"+input;
        }
        if(input.length()/2 < this->bytes.size())
        {
            size_t emptyBytes = this->bytes.size() - input.length()/2;
            input = std::string(emptyBytes*2,'0')+input;
        }
        for(size_t i=0 ; i<this->bytes.size() ; i++)
        {
            this->bytes[i] = (uint8_t)strtoul(input.substr(2*i,2).c_str(), nullptr, 16);
        }
    }
    bool operator < (const Bits<T>& rhs)
    {
        bool result = true;
        for(size_t i=0 ; i<this->bytes.size() ; i++)
        {
            if(this->bytes[i] > rhs.bytes[i])
            {
                result = false;
                break;
            }
            else if(this->bytes[i] < rhs.bytes[i])
            {
                result = true;
                break;
            }
        }
        return result;
    }
};

template <size_t T> class Bytes
{
protected:
    std::array<uint8_t,T> bytes;
public:
    Bytes(){ this->reset(); };
    Bytes(const Bytes<T>& ref){ this->bytes = ref.bytes; };
    
    uint8_t* ptr(size_t offset = 0) {return ((uint8_t*)this->bytes.data())+offset;};
    uint16_t* ptr16(size_t offset = 0) {return ((uint16_t*)this->bytes.data())+offset;};
    uint32_t* ptr32(size_t offset = 0) {return ((uint32_t*)this->bytes.data())+offset;};
    uint64_t* ptr64(size_t offset = 0) {return ((uint64_t*)this->bytes.data())+offset;};
    
    const uint8_t* ptr(size_t offset = 0) const {return ((const uint8_t*)this->bytes.data())+offset;};
    const uint16_t* ptr16(size_t offset = 0) const {return ((const uint16_t*)this->bytes.data())+offset;};
    const uint32_t* ptr32(size_t offset = 0) const {return ((const uint32_t*)this->bytes.data())+offset;};
    const uint64_t* ptr64(size_t offset = 0) const {return ((const uint64_t*)this->bytes.data())+offset;};
    
    void reset(){ std::fill( std::begin( this->bytes ), std::end( this->bytes ), 0 ); };
    
    uint16_t value16(size_t index) const { return ((uint16_t*)this->ptr())[index]; };
    uint32_t value32(size_t index) const { return ((uint32_t*)this->ptr())[index]; };
    uint64_t value64(size_t index) const { return ((uint64_t*)this->ptr())[index]; };
    template <typename V> V value(size_t index){
        return ((V*)this->ptr())[index];
    }
    
    void value16(size_t index, uint16_t value) { ((uint16_t*)this->ptr())[index] = value; };
    void value32(size_t index, uint32_t value) { ((uint32_t*)this->ptr())[index] = value; };
    void value64(size_t index, uint64_t value) { ((uint64_t*)this->ptr())[index] = value; };
    template <typename V> void value(size_t index, V value){
        ((V*)this->ptr())[index] = value;
    }
    
    static const size_t bitsSize(){ return T*8; };
    static const size_t byteSize(){ return T; };
};

#endif /* bits_h */
