#!/usr/bin/python3
# @file check_code.py
# @brief This program applies a cppcheck program to project source code.
#
# This is a Python program used to check the project source code using the
# cppcheck utility.
#
# @revs_title
# @revs_begin
# @rev_entry{ James Gentile, NASA ER7, project, July 2023, --, Initial creation based off of project scripts.}
# @revs_end
#
import sys
import time
import os
import argparse
import textwrap
import git
import xml.etree.ElementTree as ET
import subprocess
import yaml
from glob import glob

from ci_message import *
from ci_utils import *

# Establish globals and set defaults for CI variables.
project_repo = git.Repo('.', search_parent_directories=True)
project_home = git.Repo('.', search_parent_directories=True).working_tree_dir
project_scripts_dir = os.path.join(project_home, 'scripts')
project_models_dir = os.path.join(project_home, 'models')
project_maintenance_dir = os.path.join(project_home, 'maintenance')
system_includes = []
project_defines = []
project_include_dirs = []

# Set default global values for the cppcheck files and directories.
cppcheck_cmd = '/usr/local/bin/cppcheck'
cppcheck_ver = '2.6.1'
cppcheck_dir_path = os.path.join(project_home, 'maintenance', 'cppcheck')
cppcheck_config_dir = os.path.join(cppcheck_dir_path, 'config')
cppcheck_work_dir = os.path.join(cppcheck_dir_path, 'work')
cppcheck_output_text_file = 'cppcheck_output.txt'
cppcheck_output_xml_file = 'cppcheck_output.xml'
cppcheck_suppresion_file = 'cppcheck_suppression.txt'
cppcheck_suppresion_autogen_file = 'suppression_autogen.txt'
cppcheck_status = 0
cppcheck_config = os.path.join(project_home, '.static-code-analysis.yml')
cppcheck_htmlreport_cmd = glob('%s/**/cppcheck-htmlreport' % project_home, recursive=True)[0]

# Main routine.
def main():
   
   # Global CI variables
   global project_repo
   global project_home
   global project_scripts_dir
   global project_maintenance_dir
   global project_defines
   global project_include_dirs
   global project_models_ignore
   
   # Global Trick variables
   global trick_home
   global trick_ver
   global trick_ver_year
   
   # Global cppcheck variables.
   global cppcheck_cmd
   global cppcheck_dir_path
   global cppcheck_config_dir
   global cppcheck_work_dir
   global cppcheck_output_text_file
   global cppcheck_output_xml_file
   global cppcheck_suppresion_file
   global cppcheck_suppresion_autogen_file
   global cppcheck_status
   global cppcheck_configs

   #
   # Read configuration .yml file
   #
   with open(cppcheck_config) as file:
       config_file = yaml.safe_load(file)

       # Build up list of whitelist models
       whitelist_models = []
       if config_file['cppcheck_whitelist']:
        for model in config_file['cppcheck_whitelist']:
           whitelist_models.append(model)

       # Build up list of configs for cppcheck
       cppcheck_configs = []
       if config_file['cppcheck_configs']:
        for config in config_file['cppcheck_configs']:
            cppcheck_configs = []

       # Build up list of CFLAGS
       cppcheck_cflags = []
       if config_file.get('cppcheck_cflags'):
        for cflag in config_file['cppcheck_cflags']:
           cppcheck_cflags.append('-I'+project_home+'/'+cflag)
   #
   # Setup command line argument parsing.
   #
   parser = argparse.ArgumentParser( prog = 'check_code', \
                                     formatter_class = argparse.RawDescriptionHelpFormatter, \
                                     description = 'Check the source code using cppcheck.', \
                                     epilog = textwrap.dedent( '''\n
Options -s, -e, -u, -x: Default error suppression file: '''+ cppcheck_config_dir + '/' + cppcheck_suppresion_file + '''\n
Option -a: Autogenerated error suppression file: ''' + cppcheck_work_dir + '/<model_path>/' + cppcheck_suppresion_autogen_file + '''\n
Examples:\n  check_code -s -o -v\n  check_code -i -o -v''' ) )

   parser.add_argument( '-a', '--autogen', \
                        help = 'Autogenerate a suppression file for all issues. Use this as a starting point for a project specific suppression file.', \
                        action = 'store_true', dest = 'autogen' )
   parser.add_argument( '-b', '--bin', \
                        help = 'Path to cppcheck binaries directory.', \
                        dest = 'bin_path' )
   parser.add_argument( '-c', '--clean', \
                        help = 'Clean all generated files.', \
                        action = 'store_true', dest = 'clean_gen_files' )
   parser.add_argument( '-e', '--errors', \
                        help = 'Check source code for errors only.', \
                        action = 'store_true', dest = 'check_errors_only' )
   parser.add_argument( '-i', '--includes', help = 'Check the #include\'s.', \
                        action = 'store_true', dest = 'check_includes' )
   parser.add_argument( '-m', '--model', help = 'Select a specific model to check.', \
                        dest = 'model' )
   parser.add_argument( '-n', '--inconclusive', help = 'Allow cppcheck to report even though the analysis is inconclusive. Caution, there are false positives with this option.', \
                        action = 'store_true', dest = 'check_inconclusive' )
   parser.add_argument( '-o', '--overwrite', help = 'Overwrite generated files.', \
                        action = 'store_true', dest = 'overwrite_files' )
   parser.add_argument( '-p', '--path', help = 'Path to project directory.', \
                        dest = 'project_home' )
   parser.add_argument( '-s', '--check_all', \
                        help = 'Check source code for all issues.', \
                        action = 'store_true', dest = 'check_all' )
   parser.add_argument( '-t', '--test', \
                        help = 'Do not run cppcheck just show what would be done.', \
                        action = 'store_true', dest = 'test_only' )
   parser.add_argument( '--trick', \
                        help = 'Provide a path to the Trick installation directory.', \
                        dest = 'trick_home' )
   parser.add_argument( '-u', '--unused', \
                        help = 'Check source code for unused functions.',
                        action = 'store_true', dest = 'check_for_unused' )
   parser.add_argument( '-v', '--verbose', \
                        help = 'Check source code with verbose detailed error information.', \
                        action = 'store_true', dest = 'verbose' )
   parser.add_argument( '-vv', '--very_verbose', \
                        help = 'Run with both cppcheck verbose output and script verbose output.', \
                        action = 'store_true', dest = 'very_verbose' )
   parser.add_argument( '-x', '--xml_output', \
                        help = 'Same as -s but the output is XML.', \
                        action = 'store_true', dest = 'generate_xml' )

   # Parse the command line arguments.
   args = parser.parse_args()
   
   # Check to see if we can find and setup the environment.
   if not project_home:
      CIMessage.failure('Could not find the project home directory!')
   if not project_scripts_dir:
      CIMessage.failure('Could not find the project scripts directory!')
      
   #
   # Now set and check for the necessary project directories.
   #
   # Now let's check to make sure that the project mantanence directory exist.
   project_maintenance_dir = os.path.join(project_home, 'maintenance')
   if not os.path.isdir( project_maintenance_dir ):
      CIMessage.failure( 'Could not find the project mantanence directory: ' + project_maintenance_dir)
      
   # Move into the project models directory.
   os.chdir(project_home)

   #
   # Let's do some command line option sanity checks.
   #
   arg_error = False
   # Check for 'autogen' incompatible arguments.
   if args.autogen and args.clean_gen_files:
      arg_error = True
      CIMessage.warning( 'The \'-a\' and \'-c\' options are incompatible!' )

   if args.autogen and args.check_errors_only:
      arg_error = True
      CIMessage.warning( 'The \'-a\' and \'-e\' options are incompatible!' )

   if args.autogen and args.check_for_unused:
      arg_error = True
      CIMessage.warning( 'The \'-a\' and \'-u\' options are incompatible!' )

   if args.autogen and args.check_all:
      arg_error = True
      CIMessage.warning( 'The \'-a\' and \'-s\' options are incompatible!' )

   if args.autogen and args.generate_xml:
      arg_error = True
      CIMessage.warning( 'The \'-a\' and \'-x\' options are incompatible!' )

   # User must specify one of -a, -c, -e, -i, -s, -u, or -x.
   required_arg_cnt = 0
   if args.autogen:
      # -a
      required_arg_cnt += 1
   if args.clean_gen_files:
      # -c
      required_arg_cnt += 1
   if args.check_errors_only:
      # -e
      required_arg_cnt += 1
   if args.check_includes:
      # -i
      required_arg_cnt += 1
   if args.check_all:
      # -s
      required_arg_cnt += 1
   if args.check_for_unused:
      # -u
      required_arg_cnt += 1
   if args.generate_xml:
      # -x
      required_arg_cnt += 1
   if required_arg_cnt == 0:
      arg_error = True
      CIMessage.warning( 'You must specify one of \'-a\', \'-c\', \'-e\', \'-i\', \'-s\', \'-u\' or \'-x\'!' )
   elif required_arg_cnt >= 2:
      arg_error = True
      CIMessage.warning( 'Only specify one of \'-a\', \'-c\', \'-e\', \'-i\', \'-s\', \'-u\' or \'-x\'!' )

   if arg_error:
      CIMessage.failure( 'Error detected in parsing command arguments!' )

   # If we selected very verbose, then we also need to set verbose in the cppcheck output.
   if args.very_verbose:
      args.verbose = True
   
   #
   # Let's make sure that we can find Trick, JEOD, cppcheck.
   #
   # Determine the path to Trick HOME and the Trick version ID.
   trick_home, trick_ver, trick_ver_year = find_trick( verbose = args.very_verbose )
   if args.very_verbose:
      CIMessage.status( 'Path to Trick: ' + trick_home )
      CIMessage.status( 'Trick Version: ' + trick_ver )
      CIMessage.status( 'Trick Version Year: ' + trick_ver_year )

   # Find the cppcheck command and get the cppcheck version ID.
   cppcheck_cmd, cppcheck_ver = find_cppcheck( args.bin_path, args.very_verbose )
   if cppcheck_cmd is None:
      CIMessage.failure( 'No cppcheck command found!' )
   
   # Build the cppcheck directory paths.
   cppcheck_dir_path = os.path.join(project_home, 'maintenance', 'cppcheck')
   cppcheck_config_dir = os.path.join(cppcheck_dir_path, 'config')
   cppcheck_work_dir = os.path.join(cppcheck_dir_path, 'work')

   # Now let's check to make sure that the project cppcheck directories exist.
   if not os.path.isdir( cppcheck_dir_path ):
      CIMessage.failure( 'Could not find the project cppcheck directory: ' + cppcheck_dir_path)
   if not os.path.isdir( cppcheck_config_dir ):
      CIMessage.failure( 'Could not find the project cppcheck config directory: ' + cppcheck_config_dir)
   if not os.path.isdir( cppcheck_work_dir ):
      CIMessage.failure( 'Could not find the project cppcheck work directory: ' + cppcheck_work_dir)

   # Define preprocessor symbols we use for the project and set the TRICK_VER based on the
   # version of the Trick simulation environment we found in our Path.
   project_defines = ['-DTRICK_VER=' + trick_ver_year ]

   # Echo check the data if verbose is selected.
   if args.verbose:
      CIMessage.status( '---------------------------------------------------------------------' )
      CIMessage.status( 'project_home            = ' + project_home )
      CIMessage.status( 'project_scripts_dir     = ' + project_scripts_dir )
      CIMessage.status( 'project_maintenance_dir = ' + project_maintenance_dir )
      CIMessage.status( 'project_defines         = ' + ' '.join( project_defines ) )
      CIMessage.status( 'project_include_dirs    = ' + ' '.join( project_include_dirs ) )
      CIMessage.status( 'trick_home               = ' + trick_home )
      CIMessage.status( 'cppcheck_cmd             = ' + cppcheck_cmd )
      CIMessage.status( 'cppcheck_ver             = ' + cppcheck_ver )
      CIMessage.status( 'cppcheck_config_dir      = ' + cppcheck_config_dir )
      CIMessage.status( '---------------------------------------------------------------------' )
      
   # Check to see if we are just cleaning up.
   if args.clean_gen_files:
      if clean_directory( cppcheck_work_dir, {'.gitkeep'}, args.test_only, args.very_verbose ):
         CIMessage.success( 'Finished cleaning up project cppcheck artifacts.' )
      else:
         CIMessage.failure( 'Error cleaning up project cppcheck artifacts.' )
         
      # No need to proceed from here.  Just return.
      return
   
   #
   # Start checking project models.
   #
   # Check to see if we are doing a specific model.
   if args.model:
      
      # Let the user know what's going on .
      if args.verbose:
         CIMessage.status('Checking project model: ' + args.model)
         
      # Check the model.
      error = check_model( args.model, args, cppcheck_cflags )
      if error:
         CIMessage.error('Error detected check model: ' + args.model)   
      else:
         CIMessage.success('Finished checking project source code: ' + model)
      
   else:
   
      # Build the list of models
      project_models = []
      project_models_ignore = []
      project_files = project_repo.git.execute(['git', 'ls-tree', '-r', 'HEAD', '--name-only']).split()
      for file in project_files:
         if file.find("/src") > -1 and file.find("/data") < 0 and file.find("/maintenance") < 0 and file.find("/html") < 0 and file.find("/apps") < 0:
            model_dir = file.split("/src")[0].split("/include")[0]
            if model_dir not in project_models:
               project_models.append(model_dir)
         elif file.find("/data") > -1:
             project_models_ignore.append(os.path.dirname(file))

      if config_file.get('cppcheck_model_dirs'):
         for model_dir in config_file['cppcheck_model_dirs']:
             project_models.append(model_dir)

      # Let the user know we're check all the models.
      if args.verbose:
         CIMessage.status('Checking all project models.')
      
      # Iterate through the list of models.
      for model in project_models:
            
         # Let the user know that we are checking a specific model.
         if args.verbose:
            CIMessage.status('Checking project model: ' + model)
         
         # Check the model.
         if model not in whitelist_models:
             error = check_model( model, args, cppcheck_cflags )
             if error:
                CIMessage.error('Error detected check model: ' + model)
             else:
                CIMessage.success('Finished checking project source code: ' + model)

             # Set the Overall Status
             if error:
                 cppcheck_status = 1
         else:
             CIMessage.warning('Model has been marked to skip cppcheck: ' + model)

      # End of model list interation.
      
   # End of check for specific model check.

   # Echo check the data if verbose is selected.
   if args.verbose:
      CIMessage.status( '---------------------------------------------------------------------' )
      CIMessage.status( 'project_home            = ' + project_home )
      CIMessage.status( 'project_scripts_dir     = ' + project_scripts_dir )
      CIMessage.status( 'project_maintenance_dir = ' + project_maintenance_dir )
      CIMessage.status( 'project_models_dir      = ' + project_models_dir )
      CIMessage.status( 'project_defines         = ' + ' '.join( project_defines ) )
      CIMessage.status( 'project_include_dirs    = ' + ' '.join( project_include_dirs ) )
      CIMessage.status( 'trick_home               = ' + trick_home )
      CIMessage.status( 'cppcheck_cmd             = ' + cppcheck_cmd )
      CIMessage.status( 'cppcheck_ver             = ' + cppcheck_ver )
      CIMessage.status( 'cppcheck_config_dir      = ' + cppcheck_config_dir )
      CIMessage.status( '---------------------------------------------------------------------' )
      
   return cppcheck_status

# Function to cppcheck a specific model directory.
#
# This function will check a specific model.
#
# @return error    The path to the cppcheck command.
# @param  model    The path to the cppcheck programs directory.
# @param  args     The argument list passed to the top level command.
#
def check_model( model, args, cflaglist):
   
   # Global variables.
   global trick_home
   global cppcheck_config_dir
   global cppcheck_work_dir
   
   # Working variables.
   project_ignore = []
   cppcheck_args = []
   status = 0
   
   # Build the full path to the model and make sure that it exists.
   full_model_path = os.path.join(project_home, model)
   if os.path.isdir( full_model_path  ):
      if args.very_verbose:
         CIMessage.success( 'Found model: ' + full_model_path )
   else:
      CIMessage.failure( 'Could not find model: ' + full_model_path )
   
   # Build up this model's output directory.
   cppcheck_model_config_path = os.path.join(cppcheck_config_dir, model)
   cppcheck_model_work_path = os.path.join(cppcheck_work_dir, model)
   cppcheck_build_dir_path = os.path.join(cppcheck_model_work_path, 'build')
   
   # Build up this model's suppression file paths.
   project_supression_file = os.path.join(cppcheck_config_dir, cppcheck_suppresion_file)
   model_supression_file = os.path.join(cppcheck_model_config_path, cppcheck_suppresion_file)

   # Create the cppcheck output directory.
   if os.path.isdir( cppcheck_model_work_path ):
      if args.overwrite_files:
         if args.very_verbose:
            CIMessage.status( 'Directory \'' + cppcheck_model_work_path + '\' exists!' )
      else:
         CIMessage.failure( 'Directory \'' + cppcheck_model_work_path + '\' exists!' )
   else:
      if args.test_only:
         CIMessage.status( 'Would create the \'' + cppcheck_model_work_path + '\' directory.' )
      else:
         if args.very_verbose:
            CIMessage.status( 'Creating \'' + cppcheck_model_work_path + '\' directory.' )
         # Create the cppcheck output directory.
         os.makedirs( cppcheck_model_work_path )

   # Create the cppcheck/build directory.
   if os.path.isdir( cppcheck_build_dir_path ):
      if args.overwrite_files:
         if args.very_verbose:
            CIMessage.status( 'Directory \'' + cppcheck_build_dir_path + '\' exists!' )
      else:
         CIMessage.failure( 'Directory \'' + cppcheck_build_dir_path + '\' exists!' )
   else:
      if args.test_only:
         CIMessage.status( 'Would create the \'' + cppcheck_build_dir_path + '\' directory.' )
      else:
         if args.very_verbose:
            CIMessage.status( 'Creating \'' + cppcheck_build_dir_path + '\' directory.' )
         # Create the cppcheck build directory.
         os.makedirs( cppcheck_build_dir_path )

   #
   # Let's start building the cppcheck command arguments.
   #
   # While this should not happen, make sure Trick and JEOD are ignored.
   project_ignore.append( '-i' + trick_home )
   for file in project_models_ignore:
      project_ignore.append( '-i' + file )
      
   # Check to see if the model has an ignore file.
   model_ignore_module = cppcheck_model_config_path + '/cppcheck_ignore.py'
   if os.path.isfile(model_ignore_module):
      if args.test_only:
         if args.very_verbose:
            CIMessage.status( 'Would load the cppcheck_ignore.py module: ' + model_ignore_module )
         if args.verbose:
            CIMessage.status( 'Would load the cppcheck_ignore.py module for model: ' + model )
      else:
         if args.very_verbose:
            CIMessage.status( 'Loading the cppcheck_ignore.py module: ' + model_ignore_module )
         if args.verbose:
            CIMessage.status( 'Loading the cppcheck_ignore.py module for model: ' + model )
         # Dynamic execute the models ignore specification.
         exec(open(model_ignore_module).read())

   # Set the cppcheck arguments based on the type of code checking the user wants to do.
   if args.check_errors_only:
      # We don't need to set any specific arguments for the default error checking.
      None
   elif args.check_for_unused:
      # Only look for unused functions.
      cppcheck_args.append( '--enable=unusedFunction' )
   elif args.check_includes:
      # Do we check all the #include's.
      cppcheck_args.append( '--check-config' )
      cppcheck_args.append( '--suppress=missingIncludeSystem' )
   elif args.check_all:
      # Check for everything except unused functions.
      cppcheck_args.append( '--enable=warning,style,performance,portability,information,missingInclude' )
   elif args.autogen:
      # Check for everything including unused functions for the auto-generated
      # suppression list.
      cppcheck_args.append( '--enable=all' )
   else:
      # Otherwise check everything.
      cppcheck_args.append( '--enable=all' )

   if args.verbose:
      # Enable more verbose detailed error information.
      cppcheck_args.append( '-v' )

   if args.check_inconclusive:
      # From cppcheck usage: Allow cppcheck to report even though the analysis
      # is inconclusive. There are false positives with this option. Each result
      # must be carefully investigated before you know if it is good or bad.
      cppcheck_args.append( '--inconclusive' )

   # Use the c++11 standard
   cppcheck_args.append( '--std=c++11' )

   cppcheck_args.append( '-I'+trick_home+'/include')

   cppcheck_args.append( '-I'+trick_home+'/include/trick/compat')

   # Configure cppcheck to use an output directory to cache build results.
   cppcheck_args.append( '--cppcheck-build-dir=' + cppcheck_build_dir_path )

   for cflag in cflaglist:
      cppcheck_args.append( cflag )

   # If the -s, -e or -x options are specified use the parallel processing -j option.
   if args.check_all or args.check_errors_only or args.generate_xml:
      # Enable parallel cppcheck processing based on the total number of real CPU cores
      # the computer has.
      # Get the number of CPUs on the system.
      num_cpus = get_number_of_cpus()
      if args.very_verbose:
         CIMessage.status( 'Number of CPUs: ' + str( num_cpus ) )
      # Add the multi-processor parallelization arguments.
      cppc_loadmax = None
      if num_cpus > 1:
         cppc_loadmax = ( 3 * num_cpus ) / 2
         # Set the number of cppcheck job threads and limit the load.
         cppcheck_args.extend( ['-l', str( int(cppc_loadmax) )] )

   # All commands need these paths.
   if not args.very_verbose:
      cppcheck_args.append( '--quiet' )
   cppcheck_args.append( '--force' )
   cppcheck_args.extend( system_includes )
   cppcheck_args.extend( project_ignore )
   cppcheck_args.extend( project_defines )
   cppcheck_args.extend( project_include_dirs )
   cppcheck_args.append( full_model_path )

   # Add the header files to checks list
   for filename in glob('%s/**/*.hh' % full_model_path, recursive=True):
      cppcheck_args.append( filename )

   # If we are running cppcheck on the source then use a suppression file if it exists.
   if args.check_all or args.check_errors_only or args.check_for_unused or args.generate_xml:
      # First check for a model suppression file.
      if os.path.isfile( model_supression_file ):
         cppcheck_args.insert( 0, '--suppressions-list=' + model_supression_file )
      # Otherwise, check for a project wide suppression file.
      elif os.path.isfile( project_supression_file ):
         cppcheck_args.insert( 0, '--suppressions-list=' + project_supression_file )

   # Enable XML output.
   if args.generate_xml:
      cppcheck_args.insert( 0, '--xml-version=2' )

   # Check if auto-generating a suppression file.
   if args.autogen:
      # NOTE: Don't enable inline-suppressions so we get all issues in the file.

      # Set the output file.
      #output_file = cppcheck_output_dir_path + '/' + cppcheck_suppresion_autogen_file
      output_file = cppcheck_suppresion_autogen_file
      cppcheck_args.append( '--output-file=' + output_file )

      # Define the output format template.
      cppcheck_args.insert( 0, '--template={id}:{file}:{line}' )

   else:

      # Enable inline suppressions.
      cppcheck_args.append( '--inline-suppr' )

      # Set the output file.
      if args.generate_xml:
         output_file = cppcheck_model_work_path + '/' + cppcheck_output_xml_file
      else:
         output_file = cppcheck_model_work_path + '/' + cppcheck_output_text_file

      cppcheck_args.append( '--output-file=' + output_file )

   # Add cppcheck args specified in config yml file
   for arg in cppcheck_configs:
      cppcheck_args.append(arg)

   # Echo check the data if verbose is selected.
   if args.verbose:
      CIMessage.status( '---------------------------------------------------------------------' )
      CIMessage.status( 'cppcheck_model_work_path = ' + cppcheck_model_work_path )
      CIMessage.status( 'cppcheck_build_dir_path  = ' + cppcheck_build_dir_path )
      CIMessage.status( 'cppcheck_args            = ' + ' '.join( cppcheck_args ) )
      CIMessage.status( '---------------------------------------------------------------------' )

   # Form the cppcheck command with command line options.
   shell_command = [ cppcheck_cmd ]
   shell_command.extend( cppcheck_args )

   #
   # Execute the cppcheck command
   #
   if args.test_only:
      CIMessage.status( 'Would execute: ' + ' '.join( shell_command ) )
   else:
      if args.very_verbose:
         CIMessage.status( 'Executing: ' + ' '.join( shell_command ) )

      # Execute the cppcheck command.
      try:

         # Spawn off the cppcheck process using Popen.
         cppcheck_proc = subprocess.Popen( shell_command )

         # Use spinner to indicate activity.
         count = 0
         while cppcheck_proc.poll() is None:

            # Spinner.
            if count % 4 == 0:
               sys.stdout.write( '\b' )
               sys.stdout.write( '|' )
               sys.stdout.flush()
            if count % 4 == 1:
               sys.stdout.write( '\b' )
               sys.stdout.write( '/' )
               sys.stdout.flush()
            if count % 4 == 2:
               sys.stdout.write( '\b' )
               sys.stdout.write( '-' )
               sys.stdout.flush()
            if count % 4 == 3:
               sys.stdout.write( '\b' )
               sys.stdout.write( '\\' )
               sys.stdout.flush()
            count += 1

            # Check process activity 10 times a second.
            time.sleep( 0.1 )

         # Clear the spinner line.
         sys.stdout.write( '\b' )

      except subprocess.CalledProcessError:
         CIMessage.error( subprocess.CalledProcessError.message )
         CIMessage.failure( '\'cppcheck\' command failed!' )

   if args.generate_xml:
      # Parse the xml output to check for errors
      tree = ET.parse(output_file)
      root = tree.getroot()
      for elem in root.iter('error'):
         if elem.attrib['severity'] == 'error':
            status = 1

      # Convert the XML output to html for readability
      devnull = open(os.devnull, 'w')
      subprocess.call([cppcheck_htmlreport_cmd,
        '--source-dir=%s' % model,
        '--title=%s' % model.split("/")[-1],
        '--file=%s' % output_file,
        '--report-dir=%s' % cppcheck_model_work_path], stdout=devnull)

   return status


# Function to find the cppcheck command.
#
# This function searches common locations for the cppcheck command.
#
# @return cppcheck_command The path to the cppcheck command.
# @return cppcheck_version The version of cppcheck.
# @param  cppcheck_bin     The path to the cppcheck programs directory.
# @param  verbose          Flag to set if verbose outputs are on.
#
def find_cppcheck( cppcheck_bin, verbose = True ):

   # Initialize the cppcheck command path.
   cppcheck_command = None
   cppcheck_version = None

   # Check to see if the path to the cppcheck binaries are set.
   if cppcheck_bin:

      # Use the command line path to set the command.
      cppcheck_command = os.path.join(cppcheck_bin, 'cppcheck')

   # cppcheck_bin is not set so lets check other options.
   else:

      # Check to see if the $CPPCHECK_HOME environment variable is defined and set.
      cppcheck_home = os.environ.get( 'CPPCHECK_HOME' )
      if cppcheck_home:

         # Check to see if CPPCHECK_HOME exists.
         if os.path.isdir( cppcheck_home ):

            # Form the cppcheck command based on CPPCHECK_HOME.
            cppcheck_command = os.path.join(cppcheck_home, 'cppcheck')

            if verbose:
               # Let the user know that we have CPPCHECK_HOME and where it is.
               CIMessage.status( 'CPPCHECK_HOME: ' + cppcheck_home )

         else:
            CIMessage.failure( 'CPPCHECK_HOME not found: ' + cppcheck_home )

      else:

         # CPPCHECK_HOME is not set so look in the standard locations for cppcheck.
         if os.path.isfile( '/usr/bin/cppcheck' ):
            cppcheck_command = '/usr/bin/cppcheck'
         elif os.path.isfile( '/usr/local/bin/cppcheck' ):
            cppcheck_command = '/usr/local/bin/cppcheck'
         elif os.path.isfile( '/opt/homebrew/bin/cppcheck' ):
            cppcheck_command = '/opt/homebrew/bin/cppcheck'

   # We're finished hunting. Now let's check for the cppcheck command.
   if cppcheck_command is None:
      CIMessage.failure( 'Could not find the cppcheck command!' )
   else:
      if not os.path.isfile( cppcheck_command ):
         CIMessage.failure( 'Could not find the cppcheck command!: '\
                                  +cppcheck_command )
      else:
         if verbose:
            CIMessage.status( 'Using cppcheck command: ' + cppcheck_command )

   #
   # Now get the cppcheck version ID tag.
   #
   try:
      cppcheck_version = str( subprocess.check_output( [cppcheck_command, '--version'] ) ).strip()
   except subprocess.CalledProcessError:
      CIMessage.error( subprocess.CalledProcessError.message )
      CIMessage.failure( '\'cppcheck --version\' command failed!: '
                               +cppcheck_command )

   return cppcheck_command, cppcheck_version

#
# Call the main function.
#
main()

#
# Return a code of nonzero to the CI piplein
#
if cppcheck_status:
    sys.exit(1)
