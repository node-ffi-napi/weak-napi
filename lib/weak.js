'use strict';
const EventEmitter = require('events');
const { WeakTag, ObjectInfo } = require('bindings')('weakref.node');
Object.setPrototypeOf(ObjectInfo, EventEmitter);
Object.setPrototypeOf(ObjectInfo.prototype, EventEmitter.prototype);

const kInfo = Symbol('kInfo');

const map = new WeakMap();

function weak(obj, callback) {
  const info = new ObjectInfo(obj, onGarbageCollect);
  const tag = new WeakTag(info);
  const list = map.get(obj);
  if (list === undefined)
    map.set(obj, [tag]);
  else
    list.push(tag);
  if (typeof callback === 'function')
    info.on('dead', callback);
  return makeProxy(info);
}
weak.create = weak;
module.exports = weak;

function onGarbageCollect() {
  this.emit('dead');
}

weak.get = function(ref) {
  return ref[kInfo].target;
};

weak.isDead = function(ref) {
  return weak.get(ref) === undefined;
};

weak.isNearDeath = function(ref) {
  return false;
};

weak.isWeakRef = function(ref) {
  return !!ref[kInfo];
};

weak.addCallback = function(ref, callback) {
  ref[kInfo].addListener('dead', callback);
};

weak.removeCallback = function(ref, callback) {
  ref[kInfo].removeListener('dead', callback);
};

weak.removeCallbacks = function(ref) {
  ref[kInfo].removeAllListeners('dead');
};

weak.callbacks = function(ref) {
  return ref[kInfo].listeners('dead');
};

const proxyHandler = {
  getPrototypeOf(info) {
    const target = info.target;
    return target === undefined ? null : Object.getPrototypeOf(target);
  },
  setPrototypeOf(info, proto) {
    const target = info.target;
    return target === undefined ? false : Object.setPrototypeOf(target, proto);
  },
  isExtensible(info) {
    return Object.isExtensible(info); // For invariant keeping
  },
  preventExtensions(info) {
    const target = info.target;
    if (target !== undefined) {
      Object.preventExtensions(target);
    }
    Object.preventExtensions(info); // For invariant keeping
    return true;
  },
  getOwnPropertyDescriptor(info, key) {
    const target = info.target;
    return target === undefined ? undefined :
        Object.getOwnPropertyDescriptor(target, key);
  },
  defineProperty(info, key, descriptor) {
    const target = info.target;
    return target === undefined ? false :
        Object.defineProperty(target, key, descriptor);
  },
  has(info, key) {
    const target = info.target;
    return target === undefined ? false : key in target;
  },
  get(info, key) {
    if (key === kInfo) return info;
    const target = info.target;
    return target === undefined ? undefined : target[key];
  },
  set(info, key, value) {
    const target = info.target;
    if (target !== undefined) target[key] = value;
    return true;
  },
  deleteProperty(info, key) {
    const target = info.target;
    if (target !== undefined) delete target[key];
    return true;
  },
  ownKeys(info) {
    const target = info.target;
    return target === undefined ? [] :
        Object.getOwnPropertyNames(target).concat(
            Object.getOwnPropertySymbols(target));
  }
};

function makeProxy(info) {
  return new Proxy(info, proxyHandler);
}
