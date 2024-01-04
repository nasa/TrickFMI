#!/usr/bin/python3

import git
import os, sys
import subprocess
import time
import filecmp
import numpy as np
import shutil
sys.path.append(os.path.join(os.environ.get('TRICK_HOME'), 'share', 'trick', 'trickops'))
from TrickWorkflow import *
from argparse import RawTextHelpFormatter

try:
    from CompareData import CompareData
    compare_data_found = False
except:
    compare_data_found = True

# Input Arguments
parser = argparse.ArgumentParser(description="Lander Simulation Input Arguments",\
                                 formatter_class=RawTextHelpFormatter)

parser.add_argument("--gen_regr_data",
                    help="""Option to generate regression data.""",
                    action='store_true')

parser.add_argument("--gen_koviz_reports",
                    help="""Option to generate koviz reports for all runs.""",
                    action='store_true')

parser.add_argument("--skip_build",
                    help="""Option to skip the build stage.""",
                    action='store_true')

parser.add_argument("--skip_comparisons",
                    help="""Option to skip the comparisons.""",
                    action='store_true')

inputArgs, unknowns = parser.parse_known_args()

# Lander Workflow Class
class LanderWorkflow(TrickWorkflow):
    def __init__(self, quiet):
        # Find the Project Top Level Directory
        self.project_dir = git.Repo('.', search_parent_directories=True).working_tree_dir

        # Set the Trick Home Directory
        trick_dir = os.environ.get('TRICK_HOME')

        # Base Class initialize, this creates internal management structures
        TrickWorkflow.__init__(self, project_top_level=self.project_dir, log_dir=os.path.join(os.getcwd(), os.path.join(self.project_dir, 'maintenance', 'trickops')),
            trick_dir=trick_dir, config_file=os.path.join(self.project_dir, "trickops.yml"), cpus=16, quiet=quiet)

        return

    def run_comparison(self):
        overall_status = 0

        # Run TrickOps Comparison using mdf5sum Check
        self.compare()

        # Save Regression Data
        if inputArgs.gen_regr_data:
            for comparison in self.get_comparisons():
                # Copy the Updated Regression Data
                shutil.copyfile(comparison.test_data, comparison.baseline_data)

        for comparison in self.get_comparisons():
            test_run_file = os.path.join(self.project_dir, comparison.test_data)
            regr_run_file = os.path.join(self.project_dir, comparison.baseline_data)
        
            if comparison.status == Job.Status.FAILED:
                if compare_data_found:
                    compare_data_instance = CompareData()
                    status = compare_data_instance.compare_data(self.project_dir, test_run_file, regr_run_file)
        
                if status == 0:
                    comparison.status = Job.Status.SUCCESS
                else:
                    comparison.status = Job.Status.FAILED
                    overall_status = 1

        return overall_status

    def run(self):
        build_jobs      = self.get_jobs(kind='build')
        run_jobs        = self.get_jobs(kind='run')
        
        if not inputArgs.skip_build:
            builds_status = self.execute_jobs(build_jobs, max_concurrent=1, header='Executing all sim builds.')
        else:
            builds_status = 0

        runs_status   = self.execute_jobs(run_jobs,   max_concurrent=1, header='Executing all sim runs.')

        if not inputArgs.skip_comparisons or inputArgs.gen_regr_data:
            compare_status = self.run_comparison()
        else:
            compare_status = 0

        # Create koviz reports for all comparisons
        if inputArgs.gen_koviz_reports:
            (koviz_jobs, failures) = self.get_koviz_report_jobs()
            # Execute koviz jobs to generate the error plots, up to four at once
            if not failures:
              self.execute_jobs(koviz_jobs, max_concurrent=4, header='Generating koviz error plots')

        # Create koviz reports for only failed comparisons
        elif compare_status:
            # Add koviz to the Path
            if os.environ["PATH"].find("koviz") < 0:
                os.environ["PATH"] = os.path.join(self.project_dir, "nexsys-ci/koviz/bin") + ":" + os.environ["PATH"]

            # OPTION 2: Get a single koviz report job by providing two directories to have koviz compare
            for comparison in self.get_comparisons():
                if comparison.status == Job.Status.FAILED:
                    (krj, failure) = self.get_koviz_report_job(os.path.dirname(comparison.test_data), os.path.dirname(comparison.baseline_data))
                    if not failure:
                      self.execute_jobs([krj], header='Generating koviz error plots')
            
        self.report()           # Print Verbose report
        self.status_summary()   # Print a Succinct summary

        # Set the Return Status
        if builds_status or runs_status or compare_status:
            status = 1
        else:
            status = 0

        return status

if __name__ == "__main__":
    sys.exit(LanderWorkflow(quiet=(True if len(sys.argv) > 1 and 'quiet' in sys.argv[1] else False)).run())

