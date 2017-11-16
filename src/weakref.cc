#include <napi.h>
#include <uv.h>
#include <assert.h>
#include <memory>

using namespace Napi;

namespace {

template<typename T>
void SetImmediate(Env env, T&& cb) {
  T* ptr = new T(std::move(cb));
  uv_check_t* check = new uv_check_t;
  check->data = static_cast<void*>(ptr);
  // TODO(addaleax): This should not need to be the default loop!
  uv_check_init(uv_default_loop(), check);
  uv_check_start(check, [](uv_check_t* check) {
    std::unique_ptr<T> ptr (static_cast<T*>(check->data));
    T cb = std::move(*ptr);
    uv_check_stop(check);
    uv_close(reinterpret_cast<uv_handle_t*>(check), [](uv_handle_t* handle) {
      delete reinterpret_cast<uv_check_t*>(handle);
    });

    try {
      cb();
    } catch (Error e) {
      // This is going to crash, but it's not like we really have a choice.
      e.ThrowAsJavaScriptException();
    }
  });
}

class ObjectInfo : public ObjectWrap<ObjectInfo> {
 public:
  ObjectInfo(const CallbackInfo& args) : ObjectWrap(args) {
    if (!args[0].IsObject())
      throw Error::New(Env(), "target should be object");
    if (!args[1].IsFunction())
      throw Error::New(Env(), "callback should be function");
    Ref();
    target_.Reset(args[0].As<Object>(), 0);
    callback_.Reset(args[1].As<Function>(), 1);
  }

  void OnFree() {
    SetImmediate(Env(), [this]() {
      HandleScope scope(Env());
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
