# weakref

weak references in node.js

## how to build

    # installs node-weakref system wide
    node-waf configure build install

## how it works

    weakref = require('weakref');
    
    var obj = weakref.weaken({val:42});
    console.error(obj); // "{ val: 42 }"
    // time passes, the garbage collector runs...
    console.error(obj); // "{}"

## more info

This is a proof of concept for [node.js issue 631](https://github.com/joyent/node/issues/631).
Check the link for details and discussion.
