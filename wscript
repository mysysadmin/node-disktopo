srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'disktopo'
#  obj.ldflags = '-L/lib -R/lib -L/usr/lib/fm -R/usr/lib/fm -ltopo -lnvpair'
#  obj.lddflags = '-L/lib -R/lib -L/usr/lib/fm -R/usr/lib/fm -ltopo -lnvpair'
  obj.ldflags = '-L/usr/lib/fm -R/usr/lib/fm -ltopo'
  obj.source = 'disktopo.cc'
