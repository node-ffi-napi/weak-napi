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

#include "v8.h"
#include "node.h"

using namespace v8;

namespace {


Persistent<ObjectTemplate> proxyClass;


bool IsDead(Handle<Object> proxy) {
  assert(proxy->InternalFieldCount() == 2);
  Persistent<Object>* target = reinterpret_cast<Persistent<Object>*>(
      proxy->GetPointerFromInternalField(0));
  assert(target != NULL);
  return target->IsEmpty();
}


Handle<Object> Unwrap(Handle<Object> proxy) {
  assert(!IsDead(proxy));
  Persistent<Object>* target = reinterpret_cast<Persistent<Object>*>(
      proxy->GetPointerFromInternalField(0));
  assert(target != NULL);
  return *target;
}

Handle<Array> GetCallbacks(Handle<Object> proxy) {

  Persistent<Array>* callbacks = reinterpret_cast<Persistent<Array>*>(
      proxy->GetPointerFromInternalField(1));

  // First time being called? Create a new Array
  if (callbacks == NULL) {
    callbacks = new Persistent<Array>();
    *callbacks = Persistent<Array>::New(Array::New());
    proxy->SetPointerInInternalField(1, callbacks);
  }

  assert(callbacks != NULL);
  return *callbacks;
}


#define UNWRAP                            \
  HandleScope scope;                      \
  Handle<Object> obj;                     \
  const bool dead = IsDead(info.This());  \
  if (!dead) obj = Unwrap(info.This());   \


Handle<Value> WeakNamedPropertyGetter(Local<String> property,
                                      const AccessorInfo& info) {
  UNWRAP
  return dead ? Local<Value>() : obj->Get(property);
}


Handle<Value> WeakNamedPropertySetter(Local<String> property,
                                      Local<Value> value,
                                      const AccessorInfo& info) {
  UNWRAP
  if (!dead) obj->Set(property, value);
  return value;
}


Handle<Integer> WeakNamedPropertyQuery(Local<String> property,
                                       const AccessorInfo& info) {
  return HandleScope().Close(Integer::New(None));
}


Handle<Boolean> WeakNamedPropertyDeleter(Local<String> property,
                                         const AccessorInfo& info) {
  UNWRAP
  return Boolean::New(!dead && obj->Delete(property));
}


Handle<Value> WeakIndexedPropertyGetter(uint32_t index,
                                        const AccessorInfo& info) {
  UNWRAP
  return dead ? Local<Value>() : obj->Get(index);
}


Handle<Value> WeakIndexedPropertySetter(uint32_t index,
                                        Local<Value> value,
                                        const AccessorInfo& info) {
  UNWRAP
  if (!dead) obj->Set(index, value);
  return value;
}


Handle<Integer> WeakIndexedPropertyQuery(uint32_t index,
                                         const AccessorInfo& info) {
  return HandleScope().Close(Integer::New(None));
}


Handle<Boolean> WeakIndexedPropertyDeleter(uint32_t index,
                                           const AccessorInfo& info) {
  UNWRAP
  return Boolean::New(!dead && obj->Delete(index));
}


Handle<Array> WeakPropertyEnumerator(const AccessorInfo& info) {
  UNWRAP
  return HandleScope().Close(dead ? Array::New(0) : obj->GetPropertyNames());
}


void AddCallback(Handle<Object> proxy, Handle<Function> callback) {
  Handle<Array> callbacks = GetCallbacks(proxy);
  callbacks->Set(Integer::New(callbacks->Length()), callback);
}


template <bool delete_target>
void WeakCallback(Persistent<Value> obj, void* arg) {
  assert(obj.IsNearDeath());
  obj.Dispose();
  obj.Clear();

  Persistent<Object>* target = reinterpret_cast<Persistent<Object>*>(arg);
  (*target).Dispose();
  (*target).Clear();
  if (delete_target) delete target;
}


Handle<Value> Create(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsObject()) {
    Local<String> message = String::New("Object expected");
    return ThrowException(Exception::TypeError(message));
  }

  Persistent<Object>* target = new Persistent<Object>();
  *target = Persistent<Object>::New(args[0]->ToObject());
  (*target).MakeWeak(target, WeakCallback<false>);

  Persistent<Object> proxy =
      Persistent<Object>::New(proxyClass->NewInstance());

  proxy->SetPointerInInternalField(0, target);
  proxy->SetPointerInInternalField(1, NULL);
  proxy.MakeWeak(target, WeakCallback<true>);

  if (args.Length() >= 2) {
    AddCallback(proxy, Handle<Function>::Cast(args[1]));
  }

  return proxy;
}

Handle<Value> Get(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsObject()
         || args[0]->ToObject()->InternalFieldCount() != 2) {
    Local<String> message = String::New("Weakref instance expected");
    return ThrowException(Exception::TypeError(message));
  }
  Local<Object> proxy = args[0]->ToObject();
  assert(proxy->InternalFieldCount() == 2);

  const bool dead = IsDead(proxy);
  if (dead) return Undefined();

  Handle<Object> obj = Unwrap(proxy);
  return scope.Close(obj);
}

Handle<Value> IsDead(const Arguments& args) {
  HandleScope scope;

  if (!args[0]->IsObject()
         || args[0]->ToObject()->InternalFieldCount() != 2) {
    Local<String> message = String::New("Weakref instance expected");
    return ThrowException(Exception::TypeError(message));
  }
  Local<Object> proxy = args[0]->ToObject();
  assert(proxy->InternalFieldCount() == 2);

  const bool dead = IsDead(proxy);
  return scope.Close(Boolean::New(dead));
}

Handle<Value> AddCallback(const Arguments& args) {
  HandleScope scope;
  if (!args[0]->IsObject()
         || args[0]->ToObject()->InternalFieldCount() != 2) {
    Local<String> message = String::New("Weakref instance expected");
    return ThrowException(Exception::TypeError(message));
  }
  Local<Object> proxy = args[0]->ToObject();
  assert(proxy->InternalFieldCount() == 2);

  AddCallback(proxy, Handle<Function>::Cast(args[1]));

  return Undefined();
}

Handle<Value> Callbacks(const Arguments& args) {
  HandleScope scope;
  if (!args[0]->IsObject()
         || args[0]->ToObject()->InternalFieldCount() != 2) {
    Local<String> message = String::New("Weakref instance expected");
    return ThrowException(Exception::TypeError(message));
  }
  Local<Object> proxy = args[0]->ToObject();
  assert(proxy->InternalFieldCount() == 2);

  return scope.Close(GetCallbacks(proxy));
}


void Initialize(Handle<Object> target) {
  HandleScope scope;

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
  proxyClass->SetInternalFieldCount(2);

  NODE_SET_METHOD(target, "get", Get);
  NODE_SET_METHOD(target, "create", Create);
  NODE_SET_METHOD(target, "isDead", IsDead);
  NODE_SET_METHOD(target, "callbacks", Callbacks);
  NODE_SET_METHOD(target, "addCallback", AddCallback);

}

} // anonymous namespace

NODE_MODULE(weakref, Initialize);
