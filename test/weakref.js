'use strict';
const assert = require('assert');
const weak = require('../');

describe('Weakref', function() {
  afterEach(gc);

  it('weak() should return a `Weakref` instance', function() {
    var ref = weak({});
    assert(weak.isWeakRef(ref));
  });

  it('should proxy named gets to the target', function() {
    const o = { foo: 'bar' };
    const r = weak(o);
    assert.strictEqual(r.foo, 'bar');
  });

  it('should proxy named sets to the target', function() {
    const o = {};
    const r = weak(o);
    r.foo = 'bar';
    assert.strictEqual(r.foo, 'bar');
  });

  it('should proxy named deletes to the target', function() {
    const o = { foo: 'bar' };
    const r = weak(o);
    delete r.foo;
    assert(!r.foo);
  });

  it('should proxy indexed gets to the target', function() {
    const a = [ 'foo' ];
    const r = weak(a)
    assert.strictEqual(1, a.length);
    assert.strictEqual(1, r.length);
    assert.strictEqual('foo', r[0]);
  });

  it('should proxy indexed sets to the target', function() {
    const a = [];
    const r = weak(a);
    assert.strictEqual(0, a.length);
    assert.strictEqual(0, r.length);
    r[0] = 'foo';
    assert.strictEqual(1, a.length);
    assert.strictEqual('foo', a[0]);
    r.push('bar');
    assert.strictEqual(2, a.length);
    assert.strictEqual('bar', a[1]);
  });

  it('should proxy indexed deletes to the target', function() {
    const a = [ 'foo' ];
    const r = weak(a);
    delete r[0];
    assert.strictEqual('undefined', typeof a[0]);
  });

  it('should proxy enumeration', function() {
    const o = { a: 'a', b: 'b', c: 'c', d: 'd' };
    const r = weak(o);
    assert.deepEqual(Object.keys(o), Object.keys(r))
  });

  it('should act like an empty object after target is gc\'d', function() {
    let o = { foo: 'bar' };
    const r = weak(o);
    o = null;
    assert.strictEqual('bar', r.foo);
    gc();
    assert(!r.foo);
    assert.strictEqual(0, Object.keys(r).length);
  });

  it('relays prototype set/gets', function() {
    const p = {};
    let o = { foo: 'bar' };
    const r = weak(o);
    Object.setPrototypeOf(r, p);
    assert.strictEqual(Object.getPrototypeOf(r), p);
  });

  it('voids prototype set/gets for gc\'ed objects', function() {
    const p = {};
    let o = { foo: 'bar' };
    const r = weak(o);
    Object.setPrototypeOf(r, p);
    assert.strictEqual(Object.getPrototypeOf(r), p);
    o = null;
    gc();
    assert.throws(() => {
      Object.setPrototypeOf(r, p);
    });
    assert.strictEqual(Object.getPrototypeOf(r), null);
  });

  it('relays extensibility restrictions', function() {
    const p = {};
    const o = { foo: 'bar' };
    const r = weak(o);
    assert(Object.isExtensible(r));
    assert(Object.isExtensible(o));
    Object.preventExtensions(r);
    assert(!Object.isExtensible(r));
    assert(!Object.isExtensible(o));
  });

  it('voids extensibility restrictions for gc\'ed objects', function() {
    const p = {};
    let o = { foo: 'bar' };
    const r = weak(o);
    assert(Object.isExtensible(r));
    assert(Object.isExtensible(o));
    o = null;
    gc();
    Object.preventExtensions(r);
    assert(!Object.isExtensible(r));
  });

  it('forwards object descriptor definitions', function() {
    const p = {};
    let o = { foo: 'bar' };
    const r = weak(o);
    assert(Object.defineProperty(r, 'foo', { value: 42 }));
    assert.strictEqual(r.foo, 42);
    assert('foo' in r);
    o = null;
    gc();
    assert.strictEqual(r.foo, undefined);
    r.foo = 9;
    assert.strictEqual(r.foo, undefined);
    assert.throws(() => {
      assert(Object.defineProperty(r, 'foo', { value: 42 }));
    });
    assert(!('foo' in r));
    assert.strictEqual(r.foo, undefined);
    assert.strictEqual(Object.getOwnPropertyDescriptor(r, 'foo'), undefined);
  });

  it('forwards property deletions', function() {
    const p = {};
    let o = { foo: 'bar' };
    const r = weak(o);
    assert.strictEqual(r.foo, 'bar');
    o = null;
    gc();
    assert.strictEqual(delete r.foo, true);
  });
});
