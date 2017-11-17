'use strict';
const assert = require('assert');
const weak = require('../');

describe('weak()', function() {
  afterEach(gc);

  describe('garbage collection callback', function() {
    it('should accept a function as second argument', function() {
      const r = weak({}, function () {});
      assert.strictEqual(1, weak.callbacks(r).length);
    });

    it('should invoke the callback before the target is gc\'d', function(done) {
      let called = false;
      weak({}, function() {
        called = true
      });
      assert(!called);
      gc();
      setImmediate(() => {
        assert(called);
        done();
      });
    });

    it('should invoke *all* callbacks in the internal "callback" Array',
       function(done) {
      const r = weak({});
      let called1 = false;
      let called2 = false;
      weak.addCallback(r, function() {
        called1 = true
      });
      weak.addCallback(r, function() {
        called2 = true
      });
      gc();
      setImmediate(() => {
        assert(called1);
        assert(called2);
        done();
      });
    });

    it('should invoke *all* callbacks from different weak references',
       function(done) {
      let obj = {};
      const r1 = weak(obj);
      const r2 = weak(obj);
      assert.strictEqual(weak.get(r1), obj);
      assert.strictEqual(weak.isDead(r1), false);
      obj = null;
      let called1 = false;
      let called2 = false;
      weak.addCallback(r1, function() {
        called1 = true
      });
      weak.addCallback(r2, function() {
        called2 = true
      });
      gc();
      setImmediate(() => {
        assert.strictEqual(weak.get(r1), undefined);
        assert.strictEqual(weak.isDead(r1), true);
        assert(called1);
        assert(called2);
        done();
      });
    });

    it('should preempt code for GC callback but not nextTick callbacks',
       function(done) {
      let calledGcCallback = false;
      let calledTickCallback = false;
      weak({}, function() {
        calledGcCallback = true
      });

      process.nextTick(function() {
        calledTickCallback = true
      });

      assert(!calledGcCallback);
      assert(!calledTickCallback);
      gc();
      setImmediate(() => {
        assert(calledGcCallback);
        assert(calledTickCallback);
        done();
      });
    });
  });
});

describe('callbacks()', function() {
  it('should return the Weakref\'s "callback" Array', function () {
    const r = weak({}, function() {});
    const callbacks = weak.callbacks(r);
    assert(Array.isArray(callbacks));
    assert.strictEqual(1, callbacks.length);
  });
});

describe('removeCallback()', function() {
  it('removed callbacks should not be called', function() {
    let called = false;
    const fn = function() { called = true };
    const r = weak({}, fn);
    weak.removeCallback(r, fn);
    gc();
    assert(!called);
  });
});

describe('removeCallbacks()', function() {
  it('removed callbacks should not be called', function() {
     let called = false;
     const fn = function() { called = true };
     const r = weak({}, fn);
     weak.removeCallbacks(r);
     gc();
     assert(!called);
  });
});

describe('isNearDeath()', function() {
  it('returns false', function() {
     assert(!weak.isNearDeath('whatever'));
  });
});
