#ifndef TIMED_MESSAGE_H
#define TIMED_MESSAGE_H

#include <sstmac/common/messages/sst_message.h>
#include <sstmac/common/timestamp.h>

namespace sstmac {

class timed_interface
{

 public:
  timed_interface() {}

  timed_interface(const timestamp& t) :
    time_(t) {
  }

  /**
   * Time getter
   * @return time field
   */
  timestamp
  time() const {
    return time_;
  }

  /**
   * Time setter
   * @param t time value
   */
  void
  set_time(const timestamp& t) {
    time_ = t;
  }

  void
  serialize_order(sprockit::serializer& ser);

 protected:
  void
  clone_into(timed_interface* cln) const {
    cln->time_ = time_;
  }

 protected:
  timestamp time_;

};

}

#endif // TIMED_MESSAGE_H

