
/**
 * On Unix machines, this script invokes `node-waf`.
 *
 * On Windows machines, this script does nothing...
 * This is because node-weak includes a precompiled binary for this module.
 */

if (process.platform == 'win32') {
  process.exit(0)
}

var spawn = require('child_process').spawn
  , command = 'node-waf clean || true; node-waf configure build'
  , build = spawn('/bin/sh', ['-c', command], { customFds: [0,1,2] })
