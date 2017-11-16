'use strict';
const assert = require('assert');
const weak = require('../');

function checkFunction(prop) {
  it(`should have a function "${prop}"`, function () {
    assert.strictEqual(typeof weak[prop], 'function');
  })
}

describe('exports', function () {
  afterEach(gc);

  it('should be a function', function() {
    assert.strictEqual(typeof weak, 'function');
  });

  checkFunction('get');
  checkFunction('create');
  checkFunction('isWeakRef');
  checkFunction('isNearDeath');
  checkFunction('isDead');
  checkFunction('callbacks');
  checkFunction('addCallback');
  checkFunction('removeCallback');
  checkFunction('removeCallbacks');

  it('should be a circular reference to "create"', function () {
    assert.strictEqual(weak, weak.create);
  });
});
