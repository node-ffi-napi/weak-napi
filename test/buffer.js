'use strict';
const assert = require('assert');
const weak = require('../');

describe('weak()', function () {
  afterEach(gc);

  describe('Buffer', function () {
    it('should invoke callback before destroying Buffer', function() {
      let called = false;
      weak(Buffer('test'), function (buf) {
        called = true;
      });

      assert(!called);
      gc();
      assert(called);
    });
  });
});
