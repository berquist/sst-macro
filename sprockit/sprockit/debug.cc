/**
Copyright 2009-2023 National Technology and Engineering Solutions of Sandia,
LLC (NTESS).  Under the terms of Contract DE-NA-0003525, the U.S. Government
retains certain rights in this software.

Sandia National Laboratories is a multimission laboratory managed and operated
by National Technology and Engineering Solutions of Sandia, LLC., a wholly
owned subsidiary of Honeywell International, Inc., for the U.S. Department of
Energy's National Nuclear Security Administration under contract DE-NA0003525.

Copyright (c) 2009-2023, NTESS

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Questions? Contact sst-macro-help@sandia.gov
*/

#include <sprockit/spkt_string.h>
#include <sprockit/errors.h>
#include <sprockit/debug.h>
#include <sprockit/basic_string_tokenizer.h>
#include <sprockit/statics.h>
#include <iostream>

DeclareDebugSlot(timestamp);
RegisterDebugSlot(timestamp, "turns on timestamps on all debug statements");

namespace sprockit {

static NeedDeletestatics<Debug> del_statics;


std::unique_ptr<DebugPrefixFxn> Debug::prefix_fxn;
DebugInt Debug::current_bitmask_;
DebugInt Debug::start_bitmask_;
std::unique_ptr<std::map<std::string, DebugInt*>> Debug::debug_ints_;
std::unique_ptr<std::map<std::string, std::string>> Debug::docstrings_;
std::map<std::string, DebugInt*>* Debug::debug_ints_init_ = nullptr;
std::map<std::string, std::string>* Debug::docstrings_init_ = nullptr;
int Debug::num_bits_assigned = 1; //the zeroth bit is reserved empty

#if SPROCKIT_ENABLE_DEBUG
debug_indent::debug_indent() : level(0)
{
  indents[0] = "";
  indents[1] = "  ";
  indents[2] = "    ";
  indents[3] = "      ";
  indents[4] = "        ";
  indents[5] = "          ";
}
#endif

void
Debug::deleteStatics()
{
}

void
Debug::turnOff(){
  current_bitmask_ = DebugInt(); //clear it
}

void
Debug::printDebugString(const std::string &str, std::ostream& os)
{
  std::string strToPrint;
  if (prefix_fxn){
    strToPrint = prefix_fxn->str() + str + "\n";
  } else {
    strToPrint = str + "\n";
  }
  os << strToPrint;
  os.flush();
}

std::string
DebugInt::toString() const {
  std::stringstream sstr;
  sstr << fields  << " ";
  std::stringstream actives;
  for (int i=0; i < 4; ++i){
    int on = fields & (1ull<<i);
    if (on){
      sstr << "1";
      //actives << " " << debug::slot_name(debug::slot(i));
    }
    else {
      sstr << "0";
    }
  }
  std::string ret = sstr.str() + actives.str();
  return ret;
}

void
Debug::turnOn(){
  current_bitmask_ = start_bitmask_;
}

void
Debug::turnOff(DebugInt& dint){
  if (dint.fields == 0){
    //was never turned on
    return;
  }
  DebugInt offer = ~dint;
  start_bitmask_ = start_bitmask_ & offer;
  current_bitmask_ = current_bitmask_ & offer;
}

void
Debug::turnOn(DebugInt& dint){
  if (dint.fields == 0){
    assignSlot(dint);
  }
  start_bitmask_ = start_bitmask_ | dint;
  current_bitmask_ = current_bitmask_ | dint;
}

void
Debug::checkInit()
{
  if (!debug_ints_){
    debug_ints_ = std::unique_ptr<std::map<std::string,DebugInt*>>(debug_ints_init_);
    debug_ints_init_ = nullptr;
  }
  if (!docstrings_){
    docstrings_ = std::unique_ptr<std::map<std::string,std::string>>(docstrings_init_);
    docstrings_init_ = nullptr;
  }
}

void
Debug::assignSlot(DebugInt& dint)
{
  checkInit();
  //has not been assigned a bitfield
  if (num_bits_assigned > MAX_DEBUG_SLOT){
    spkt_throw_printf(IllformedError,
      "Too many debug slots turned on! Max is %d", MAX_DEBUG_SLOT);
  }
  int slot = num_bits_assigned++;
  dint.init(slot);
}

void
Debug::turnOn(const std::string& str){
  checkInit();
  auto it = debug_ints_->find(str);
  if (it == debug_ints_->end()){
    spkt_throw_printf(InputError,
        "debug::turn_on: unknown debug flag %s",
        str.c_str());
  }
  turnOn(*it->second);
}

void
Debug::registerDebugSlot(const std::string& str, DebugInt* dint,
                         const std::string& docstring){
  if (!debug_ints_init_){
    debug_ints_init_ = new std::map<std::string, DebugInt*>;
    docstrings_init_ = new std::map<std::string, std::string>;
  }
  (*debug_ints_init_)[str] = dint;
  (*docstrings_init_)[str] = docstring;
}

static void
normalize_string(const std::string& thestr,
    const std::string& indent,
    std::ostream& os, int max_length)
{
  std::deque<std::string> tok;
  std::string space = " ";
  pst::BasicStringTokenizer::tokenize(thestr, tok, space);
  os << indent;
  int line_length = 0;
  for (auto& next : tok){
    if (line_length == 0){
        os << next;
        line_length += next.size();
    } else {
      line_length += next.size() + 1;
      if (line_length > max_length){
        os << "\n" << indent; // go to next line
        line_length = next.size();
      } else {
        os << " ";
      }
      os << next;
    }
  }
}

void
Debug::printAllDebugSlots(std::ostream& os)
{
  std::string indent = sprockit::sprintf("%22s", "");
  os << "Valid debug flags are:\n";
  std::map<std::string, std::string>::iterator it, end = docstrings_->end();
  for (it = docstrings_->begin(); it != end; ++it){
    const std::string& flag = it->first;
    const std::string& docstring = it->second;
    os << sprockit::sprintf("  %20s\n", flag.c_str());
    normalize_string(docstring, indent, os, 80);
    os << "\n";
  }
}



bool
Debug::slotActive(const DebugInt& allowed){
  DebugInt bitmask = current_bitmask_ & allowed;
  return bool(bitmask);
}

DebugPrefixFxn::~DebugPrefixFxn()
{
}


}
