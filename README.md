node-weak
=========
### Make weak references to JavaScript Objects with an associated Callback function before GC.

On certain rarer occasions, you run into the need to be notified when a JavaScript
object is going to be garbage collected. This feature is exposed to V8's C++ API,
but not to JavaScript.

That's where `node-weak` comes in! This module exports a single function,
`weak()`, which allows you to attach a callback Function to any arbitrary JS
object. The callback function will be invoked right before the Object is destroyed
and garbage collected (i.e. after there are no more remaining references to the
object in JS-land).

This module can, for example, be used for debugging; to determine whether or not
an Object is beging garbage collected as it should.
Take a look at the example below for another simple use-case.


Installation
------------

Install with `npm`:

``` shell
$ npm install weak
```


Example
-------

Here's an example of calling a `cleanup()` function on a Object before it gets
garbage collected:

``` js
var weak = require('weak')

// we are going to "monitor" this Object an invoke "cleanup" before
// the object is garbage collected
var obj = {
  , a: true
  , foo: 'bar'
  , cleanup: function () {
      delete this.a
      delete this.foo
    }
}

// Here's where we set up the weak reference
weak(obj, function () {
  // `this` inside the callback is the 'obj'. DO NOT store any new references
  // to the object, and DO NOT use the object in any async functions.
  this.cleanup()
})


// Clear out any references to the object, so that it will be GC'd at some point...
obj = null
```


API
---

### weak(Object obj, Function callback)

This module only exports a single function, `weak()`. It accepts the Object that
should be monitored, and a callback Function that will be invoked before the
Object is garbage collected. Both arguments are mandatory.

Yes, you can call `weak()` on the same object multiple times, however the order
that the callbacks will be made changes between versions of V8's so don't depend
on the exection order.
