import os

env = Environment( ENV = os.environ )
env.Append(CCFLAGS = ['-O2', '-Wall', '-g', '-std=c99', '-D_FILE_OFFSET_BITS=64'])
env.Append(CPPPATH = ['.'])

env.Program("divinefs", Split("main.c scan.c fs/ext.c fs/linuxswap.c fs/ffs.c fs/reiserfs.c fs/btrfs.c fs/ntfs.c fs/fat.c fs/hfsplus.c format.c util.c cachedread.c"))

