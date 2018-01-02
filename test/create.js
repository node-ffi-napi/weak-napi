'use strict';
const assert = require('assert');
const weak = require('../');

describe('create()', function() {
  afterEach(gc)

  it('should throw on non-"object" values', function() {
    [
      0,
      0.0,
      true,
      false,
      null,
      undefined,
      'foo',
      Symbol(),
    ].forEach((val) => {
      assert.throws(function() {
        weak.create(val);
      });
    });
  });

  it('should accept "object" values', function() {
    [
      {},
      function() {},
      () => {},
      [],
      Buffer(''),
      new ArrayBuffer(10),
      new Int32Array(new ArrayBuffer(12)),
      Promise.resolve(),
      new WeakMap(),
    ].forEach((val) => {
      assert.doesNotThrow(() => {
        assert.ok(weak.create(val));
      }, Error, String(val));
    });
  });
});
