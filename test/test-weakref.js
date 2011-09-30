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

if (typeof gc === 'undefined') {
  console.error('Run this script with `node --expose-gc ' + process.argv[1] + '`');
  process.exit(1);
}

force_gc = gc;
weakref = require('../');
assert = require('assert');

[ 0,
  0.0,
  true,
  false,
  null,
  undefined
].forEach(function(val) {
  assert.throws(function() {
    weakref.weaken(val);
  });
});

(function() {
  var obj = (function() {
    var obj = weakref.weaken({val:42});
    assert.equal(obj.val, 42);
    return obj;
  })();

  assert.equal(weakref.callbacks(obj).length, 0)
  weakref.addCallback(obj, function (obj) {
    //console.error('obj being GC\'d.')
    assert.equal(obj.val, 42);
    assert.deepEqual(obj, {val:42});
  })
  assert.equal(weakref.callbacks(obj).length, 1)

  assert.equal(obj.val, 42);
  assert.deepEqual(obj, {val:42});
  assert.ok(!weakref.isDead(obj));
  force_gc();
  assert.equal(obj.val, undefined);
  assert.deepEqual(obj, {});
  assert.ok(weakref.isDead(obj));
})();

force_gc();
