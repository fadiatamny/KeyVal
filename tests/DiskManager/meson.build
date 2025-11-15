disk_srcs = [
  'DiskManager.test.cpp',
  '../../src/models/DiskManager/DiskManager.cpp',
]

diskManagerTest = executable(
  'DiskManagerTest',
  disk_srcs,
  include_directories : src_inc,
)

test('diskmanager', diskManagerTest)
