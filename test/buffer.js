'use strict';
const assert = require('assert');
const weak = require('../');

const tick = (n, cb) => n === 0 ? cb() : setImmediate(tick, n-1, cb);

describe('weak()', function () {
  afterEach(gc);

  describe('Buffer', function () {
    it('should invoke callback before destroying Buffer', function(done) {
      let called = false;
      weak(Buffer('test'), function (buf) {
        called = true;
      });

      assert(!called);
      gc();
      tick(3, () => {
        assert(called);
        done();
      });
    });
  });
});
