
function requireTry () {

  var i = 0
    , l = arguments.length
    , n

  for (; i<l; i++) {
    n = arguments[i]
    try {
      return require(n)
    } catch (e) {
      if (!/not find/i.test(e.message)) {
        throw e
      }
    }
  }

  throw new Error('Could not load any possibilities')
}

module.exports = requireTry
