bufferpool_srcs = [
  'BufferPool.test.cpp',
  '../../src/models/BufferPool/BufferPool.cpp',
  '../../src/models/DiskManager/DiskManager.cpp',
]

bufferPoolTest = executable(
  'BufferPoolTest',
  bufferpool_srcs,
  include_directories : src_inc,
)

test('bufferpool', bufferPoolTest)