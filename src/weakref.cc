#include <napi.h>
#include <setimmediate.h>

using namespace Napi;

namespace {

class ObjectInfo : public ObjectWrap<ObjectInfo> {
 public:
  ObjectInfo(const CallbackInfo& args) : ObjectWrap(args) {
    if (!args[0].IsObject() && !args[0].IsFunction())
      throw Error::New(Env(), "target should be object");
    if (!args[1].IsFunction())
      throw Error::New(Env(), "callback should be function");
    Ref();
    target_.Reset(args[0].As<Object>(), 0);
    callback_.Reset(args[1].As<Function>(), 1);
  }

  void OnFree() {
    SetImmediate(Env(), [this]() {
      callback_.MakeCallback(Value(), {});
      callback_.Reset();
      Reset();
    });
  }

  Napi::Value GetTarget(const CallbackInfo&) {
    return target_.IsEmpty() ? Env().Undefined() : target_.Value();
  }

  static Function GetClass(Napi::Env env) {
    return DefineClass(env, "ObjectInfo", {
      ObjectInfo::InstanceAccessor("target", &ObjectInfo::GetTarget, nullptr),
    });
  }

  ObjectReference target_;
  FunctionReference callback_;
};

class WeakTag : public ObjectWrap<WeakTag> {
 public:
  WeakTag(const CallbackInfo& args) : ObjectWrap(args) {
    if (args[0].IsObject())
      info_ = ObjectInfo::Unwrap(args[0].As<Object>());
    if (info_ == nullptr)
      throw Error::New(Env(), "First argument needs to be ObjectInfo");
  }

  ~WeakTag() {
    if (info_ != nullptr)
      info_->OnFree();
  }

  static Function GetClass(Napi::Env env) {
    return DefineClass(env, "WeakTag", {});
  }

  ObjectInfo* info_ = nullptr;
};

Object Init(Env env, Object exports) {
  exports["WeakTag"] = WeakTag::GetClass(env);
  exports["ObjectInfo"] = ObjectInfo::GetClass(env);
  return exports;
}

} // anonymous namespace

NODE_API_MODULE(weakref, Init)
