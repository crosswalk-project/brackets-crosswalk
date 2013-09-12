#ifndef BRACKETS_EXTENSION_H
#define BRACKETS_EXTENSION_H

#include "XW_Extension.h"
#include "picojson.h"

struct BracketsExtension {
public:
  BracketsExtension();

  typedef void (*HandlerFunc)(const picojson::object& input,
                              picojson::object& output);
  HandlerFunc GetHandler(const std::string& command);

private:
  typedef std::map<std::string, HandlerFunc> HandlerMap;
  HandlerMap handler_map_;

  void InitializeHandlerMap();
};

#endif  // BRACKETS_EXTENSION_H
