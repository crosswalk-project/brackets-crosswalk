#ifndef BRACKETS_EXTENSION_H
#define BRACKETS_EXTENSION_H

#include "cameo_extension_public.h"
#include "picojson.h"

struct BracketsExtensionContext;

struct BracketsExtension : public CCameoExtension {
private:
  static CCameoExtensionContext* CreateContext(CCameoExtension* self);
  static const char* GetJavascript(CCameoExtension* self);
  static void Shutdown(CCameoExtension* self);

  void InitializeHandlerMap();

  typedef void (*HandlerFunc)(const picojson::object& input,
                              picojson::object& output);
  HandlerFunc GetHandler(const std::string& command);

  typedef std::map<std::string, HandlerFunc> HandlerMap;
  HandlerMap handler_map_;

public:
  BracketsExtension();

  friend struct BracketsExtensionContext;
};

struct BracketsExtensionContext : public CCameoExtensionContext {
private:
  BracketsExtension *extension_;

  static void HandleMessage(CCameoExtensionContext* self, const char* message);
  static void Destroy(CCameoExtensionContext* self);

public:
  BracketsExtensionContext(BracketsExtension *extension);
};

#endif  // BRACKETS_EXTENSION_H
