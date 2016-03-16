#pragma once

#include <exception>
#include <string>


namespace osc {

//! All exceptions thrown by osc client/server derive from osc::Exception
class Exception : public std::exception {
public:
  Exception::Exception()
    : mDescription("OSC exception")
  {
  }

  explicit Exception::Exception(const std::string &description)
    : mDescription(description)
  {
  }

  virtual ~Exception() throw() {}

  const char* what() const throw() override	{ return mDescription.c_str(); }

protected:
  void Exception::setDescription(const std::string &description)
  {
    mDescription = description;
  }

private:
  std::string mDescription;
};

} // namespace osc
