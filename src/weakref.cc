/*
 * Copyright (c) 2011, Ben Noordhuis <info@bnoordhuis.nl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include "node.h"
#include "nan.h"

using namespace v8;
using namespace node;

namespace {


typedef struct proxy_container {
  Persistent<Object> proxy;
  Persistent<Object> target;
  Persistent<Array>  callbacks;
} proxy_container;


Persistent<ObjectTemplate> proxyClass;


bool IsDead(Handle<Object> proxy) {
  assert(proxy->InternalFieldCount() == 1);
  proxy_container *cont = reinterpret_cast<proxy_container*>(
    NanGetInternalFieldPointer(proxy, 0)
  );
  return cont == NULL || cont->target.IsEmpty();
}


Handle<Object> Unwrap(Handle<Object> proxy) {
  assert(!IsDead(proxy));
  proxy_container *cont = reinterpret_cast<proxy_container*>(
    NanGetInternalFieldPointer(proxy, 0)
  );
  return NanPersistentToLocal(cont->target);
}

Handle<Array> GetCallbacks(Handle<Object> proxy) {
  proxy_container *cont = reinterpret_cast<proxy_container*>(
    NanGetInternalFieldPointer(proxy, 0)
  );
  assert(cont != NULL);
  return NanPersistentToLocal(cont->callbacks);
}


#define UNWRAP                            \
  NanScope();                             \
  Handle<Object> obj;                     \
  const bool dead = IsDead(args.This());  \
  if (!dead) obj = Unwrap(args.This());   \


NAN_PROPERTY_GETTER(WeakNamedPropertyGetter) {
  UNWRAP
  NanReturnValue(dead ? Local<Value>() : obj->Get(property));
}


NAN_PROPERTY_SETTER(WeakNamedPropertySetter) {
  UNWRAP
  if (!dead) obj->Set(property, value);
  NanReturnValue(value);
}


NAN_PROPERTY_QUERY(WeakNamedPropertyQuery) {
  NanScope();
  NanReturnValue(Integer::New(None));
}


NAN_PROPERTY_DELETER(WeakNamedPropertyDeleter) {
  UNWRAP
  NanReturnValue(Boolean::New(!dead && obj->Delete(property)));
}


Handle<Value> WeakIndexedPropertyGetter(uint32_t index,
                                        const AccessorInfo& args) {
  UNWRAP
  NanReturnValue(dead ? Local<Value>() : obj->Get(index));
}


Handle<Value> WeakIndexedPropertySetter(uint32_t index,
                                        Local<Value> value,
                                        const AccessorInfo& args) {
  UNWRAP
  if (!dead) obj->Set(index, value);
  NanReturnValue(value);
}


Handle<Integer> WeakIndexedPropertyQuery(uint32_t index,
                                         const AccessorInfo& args) {
  NanScope();
  NanReturnValue(Integer::New(None));
}


Handle<Boolean> WeakIndexedPropertyDeleter(uint32_t index,
                                           const AccessorInfo& args) {
  UNWRAP
  NanReturnValue(Boolean::New(!dead && obj->Delete(index)));
}


Handle<Array> WeakPropertyEnumerator(const AccessorInfo& args) {
  UNWRAP
  NanReturnValue(dead ? Array::New(0) : obj->GetPropertyNames());
}


void AddCallback(Handle<Object> proxy, Handle<Function> callback) {
  Handle<Array> callbacks = GetCallbacks(proxy);
  callbacks->Set(Integer::New(callbacks->Length()), callback);
}


NAN_WEAK_CALLBACK(void*, TargetCallback) {
  NanScope();
  Persistent<Value> target = NAN_WEAK_CALLBACK_OBJECT;
  void* arg = NAN_WEAK_CALLBACK_DATA(void *);

  assert(target.IsNearDeath());

  proxy_container *cont = reinterpret_cast<proxy_container*>(arg);

  // invoke any listening callbacks
  uint32_t len = cont->callbacks->Length();
  Handle<Value> argv[1];
  argv[0] = target;
  for (uint32_t i=0; i<len; i++) {

    Handle<Function> cb = Handle<Function>::Cast(
        cont->callbacks->Get(Integer::New(i)));

    TryCatch try_catch;

    cb->Call(target->ToObject(), 1, argv);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
  }

  NanSetInternalFieldPointer(cont-> proxy, 0, NULL);
  cont->proxy.Dispose();
  cont->proxy.Clear();
  cont->target.Dispose();
  cont->target.Clear();
  cont->callbacks.Dispose();
  cont->callbacks.Clear();
  free(cont);
}


NAN_METHOD(Create) {
  NanScope();

  if (!args[0]->IsObject()) {
    return NanThrowTypeError("Object expected");
  }

  proxy_container *cont = (proxy_container *)
    malloc(sizeof(proxy_container));

  cont->target = Persistent<Object>::New(args[0]->ToObject());
  cont->callbacks = Persistent<Array>::New(Array::New());

  cont->proxy = Persistent<Object>::New(proxyClass->NewInstance());
  NanSetInternalFieldPointer(cont->proxy, 0, cont);

  NanMakeWeak(cont->target, cont, TargetCallback);

  if (args.Length() >= 2) {
    AddCallback(cont->proxy, Handle<Function>::Cast(args[1]));
  }

  return cont->proxy;
}

/**
 * TODO: Make this better.
 */

bool isWeakRef (Handle<Value> val) {
  return val->IsObject() && val->ToObject()->InternalFieldCount() == 1;
}

NAN_METHOD(IsWeakRef) {
  NanScope();
  return Boolean::New(isWeakRef(args[0]));
}

NAN_METHOD(Get) {
  NanScope();

  if (!isWeakRef(args[0])) {
    return NanThrowTypeError("Weakref instance expected");
  }
  Local<Object> proxy = args[0]->ToObject();

  const bool dead = IsDead(proxy);
  if (dead) return Undefined();

  Handle<Object> obj = Unwrap(proxy);
  return scope.Close(obj);
}

NAN_METHOD(IsNearDeath) {
  NanScope();

  if (!isWeakRef(args[0])) {
    return NanThrowTypeError("Weakref instance expected");
  }
  Local<Object> proxy = args[0]->ToObject();

  proxy_container *cont = reinterpret_cast<proxy_container*>(
    NanGetInternalFieldPointer(proxy, 0)
  );
  assert(cont != NULL);

  Handle<Boolean> rtn = Boolean::New(cont->target.IsNearDeath());

  return scope.Close(rtn);
}

NAN_METHOD(IsDead) {
  NanScope();

  if (!isWeakRef(args[0])) {
    return NanThrowTypeError("Weakref instance expected");
  }
  Local<Object> proxy = args[0]->ToObject();

  const bool dead = IsDead(proxy);
  NanReturnValue(Boolean::New(dead));
}


NAN_METHOD(AddCallback) {
  NanScope();

  if (!isWeakRef(args[0])) {
    return NanThrowTypeError("Weakref instance expected");
  }
  Local<Object> proxy = args[0]->ToObject();

  AddCallback(proxy, Handle<Function>::Cast(args[1]));

  NanReturnUndefined();
}

NAN_METHOD(Callbacks) {
  NanScope();

  if (!isWeakRef(args[0])) {
    return NanThrowTypeError("Weakref instance expected");
  }
  Local<Object> proxy = args[0]->ToObject();

  NanReturnValue(GetCallbacks(proxy));
}


void Initialize(Handle<Object> exports) {
  NanScope();

  proxyClass = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
  proxyClass->SetNamedPropertyHandler(WeakNamedPropertyGetter,
                                      WeakNamedPropertySetter,
                                      WeakNamedPropertyQuery,
                                      WeakNamedPropertyDeleter,
                                      WeakPropertyEnumerator);
  proxyClass->SetIndexedPropertyHandler(WeakIndexedPropertyGetter,
                                        WeakIndexedPropertySetter,
                                        WeakIndexedPropertyQuery,
                                        WeakIndexedPropertyDeleter,
                                        WeakPropertyEnumerator);
  proxyClass->SetInternalFieldCount(1);

  NODE_SET_METHOD(exports, "get", Get);
  NODE_SET_METHOD(exports, "create", Create);
  NODE_SET_METHOD(exports, "isWeakRef", IsWeakRef);
  NODE_SET_METHOD(exports, "isNearDeath", IsNearDeath);
  NODE_SET_METHOD(exports, "isDead", IsDead);
  NODE_SET_METHOD(exports, "callbacks", Callbacks);
  NODE_SET_METHOD(exports, "addCallback", AddCallback);
}

} // anonymous namespace

NODE_MODULE(weakref, Initialize);
