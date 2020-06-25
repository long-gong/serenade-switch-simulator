#!/usr/bin/env python3
import argparse
import subprocess
import yaml
import json
from copy import deepcopy
import itertools
import sys
import os.path

# parser configuration
parser = argparse.ArgumentParser()
parser.add_argument('-y', action='store', dest='filename', type=str,
                    help='Simulation description in YAML')
parser.add_argument('-o', action='store', dest='output_folder', type=str,
                    help='Directory to place output')
parser.add_argument('-j', action='store', dest='filename', type=str,
                    help='Simulation description in JSON')
parser.add_argument('-m', action='store_true', dest='multi_thread', default=False,
                    help='Whether or not to use multiple threads (TO Be Finished)')
parser.add_argument('--version', action='version', version='%(prog)s 0.1.0')

# current path
cwd = os.getcwd()

output_folder = os.path.join(cwd, 'outputs')

# templates
template_fixed = '{simulator} {fixed_args}'
template_var = ' -{opt} {val}'
template_output_redirect = ' >{output_folder}/{output_file}'


def parse_option_values(opt_values):
    return opt_values['values']


def generate_runnable_commands(simu, optional_arg='', redirect=True):
    fixed_multi_args = ''

    for s in simu.get('fixed_args_with_multiple_values', []):
        for v in s['values']:
            fixed_multi_args += template_var.format(opt=s['arg'], val=v)

    for s in simu['simulator']:
        my_sim = template_fixed.format(simulator=os.path.join(cwd, s), fixed_args=simu.get
        ('fixed_args', '')) \
                 + fixed_multi_args + optional_arg

        if redirect:
            simu_script = template_fixed.format(simulator=s.replace('/\\', '_'), fixed_args=simu['fixed_args']) \
                          + fixed_multi_args + optional_arg
            of = "".join(simu_script.split()) + ".txt"
            yield my_sim + template_output_redirect.format(output_folder=output_folder, output_file=of)
        else:
            yield my_sim


def load_experiments(simu_desp_filename, redirect=True):
    if simu_desp_filename.endswith(".yml") or simu_desp_filename.endswith(".yaml"):
        loader = yaml.safe_load
    elif simu_desp_filename.endswith(".json"):
        loader = json.load
    else:
        print("Configuration file type ", os.path.splitext(simu_desp_filename)[-1], "is not supported")
        print("Currently, only json and yaml are supported")
        exit(1)
    data = None
    with open(simu_desp_filename, 'r') as fp:
        data = loader(fp)

    for simu_id, simu in data.iteritems():

        args = []
        values = []
        for args_values in simu.get('variant_args', []):
            values.append(deepcopy(parse_option_values(args_values)))
            args.append(args_values['arg'])

        if len(values) > 0:
            for v in itertools.product(*values):
                op = ''
                for opt, val in zip(args, v):
                    op += template_var.format(opt=opt, val=val)

                for cl in generate_runnable_commands(simu, optional_arg=op, redirect=redirect):
                    yield cl
        else:
            for cl in generate_runnable_commands(simu, redirect=redirect):
                yield cl


def run_experiments(command):
    print("Experiment to run: ", command)
    subprocess.call(command, shell=True)


if __name__ == "__main__":

    args = parser.parse_args(sys.argv[1::])

    if not (args.output_folder is None):
        output_folder = args.output_folder

    if not os.path.exists(output_folder):
        os.mkdir(output_folder)

    for cl in load_experiments(simu_desp_filename=args.filename):
        run_experiments(cl)
