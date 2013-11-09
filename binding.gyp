{
  'targets': [{
    'target_name': 'weakref',
    'sources': [ 'src/weakref.cc' ],
    'include_dirs': [
      "<!(node -p -e \"require('path').relative('.', require('path').dirname(require.resolve('nan')))\")"
    ]
  }]
}
