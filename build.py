#!/usr/bin/python
import os, platform, shutil, argparse
from subprocess import call
ROOT_PATH = os.path.dirname(os.path.realpath(__file__))
os.chdir(ROOT_PATH);



# Install the application into the users bin directory
def install(args):
	build(args)
	print('install')



# Compile the application
def build(args):
	bootstrap(args)
	print('build')



# Ensure all dependencies are setup for building
def bootstrap(args):
	print('bootstrapping environment...')
	lib_dir = os.path.join(ROOT_PATH, 'lib')
	if not os.path.exists(lib_dir): os.makedirs(lib_dir)

	print('fetching vendor source...')
	call('git submodule init', shell=True)
	call('git submodule update', shell=True)

	print('building libuv...')
	libuv_root = os.path.join(ROOT_PATH, 'vendor', 'libuv')
	libuv_build = os.path.join(libuv_root, 'build')
	libuv_out = os.path.join(lib_dir, 'libuv.a')
	os.chdir(libuv_root)

	if not os.path.exists(libuv_out):
		if platform.system() == 'Windows':
			call('vcbuild.bat')
			# TODO: Run the command line version of visual studio build
		else:
			# Ensure we have gyp installed
			if not os.path.exists(libuv_build): os.makedirs(libuv_build)
			if not os.path.exists(os.path.join(libuv_build, 'gyp')):
				call('git clone https://git.chromium.org/external/gyp.git build/gyp', shell=True)

			# Build the libuv.a static library
			if platform.system() == 'Darwin':
				call('./gyp_uv.py -f xcode', shell=True)
				call('xcodebuild -ARCHS="i386 x86_64" -project uv.xcodeproj -configuration Release -target All', shell=True)
				shutil.copy(os.path.join(libuv_root, 'build', 'Release', 'libuv.a'), libuv_out)
			elif os.name =='posix':
				call('./gyp_uv.py -f make', shell=True)
				call('make -C out', shell=True)
	else:
		print('skipping libuv, output exists.')
   
   

	print('building libboost_context...')
	boost_context_root = os.path.join(ROOT_PATH, 'vendor', 'boost-context')
	boost_context_out = os.path.join(lib_dir, 'libboost_context.a')
	os.chdir(boost_context_root)

	if not os.path.exists(boost_context_out):
		if platform.system() == 'Windows':
			call('./bootstrap.bat', shell=True)
		else:
			call('./bootstrap.sh', shell=True)

		call('./b2', shell=True)
		shutil.copy(os.path.join(boost_context_root, 'stage', 'lib', 'libboost_context.a'), boost_context_out)
	else:
		print('skipping libboost_context, output exists.')



# Clean the output files
def clean(args):
	print('cleaning environment...')

	libuv_out = os.path.join(ROOT_PATH, 'lib', 'libuv.a')
	if os.path.exists(libuv_out):
		print('removing libuv...')
		os.remove(libuv_out)

	boost_context_out = os.path.join(ROOT_PATH, 'lib', 'libboost_context.a')
	if os.path.exists(boost_context_out):
		print('removing libboost_context...')
		os.remove(boost_context_out)

		boost_context_root = os.path.join(ROOT_PATH, 'vendor', 'boost-context')
		os.chdir(boost_context_root)
		call('./b2 --clean', shell=True)



if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('task', choices=['bootstrap', 'build', 'install', 'clean'])
	args = parser.parse_args()

	if args.task == 'clean': clean(args)
	if args.task == 'bootstrap': bootstrap(args)
	if args.task == 'build': build(args)
	if args.task == 'install': install(args)
	if args.task == 'package': install(args)

