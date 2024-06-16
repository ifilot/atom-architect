# -*- coding: utf-8 -*-

import jinja2
import os
import re
from datetime import date

ROOT = os.path.dirname(__file__)

def main():
    # grab_version
    version = grab_version()
    
    # set names and folders
    variables = {
        'publisher': 'IMC',
        'progfolder': 'atom-architect',
        'executable': 'atom-architect',
        'progdescription': 'Visualization and building tool for electronic structure simulations',
        'progname': 'Atom Architect',
        'version': version,
        'releasedate': str(date.today()),
        'iconfilename': 'atom_architect_256.ico',
    }
    
    templates, filenames = read_templates()
    render(templates, filenames, variables)

def render(templates, filenames, variables):
    for template, filename in zip(templates, filenames):
        content = template.render(variables)
        with open(filename, 'w', encoding='utf-8') as f:
            f.write(content)

def read_templates():
    templates = [
        os.path.join(ROOT, 'config', 'config.xml'),
        os.path.join(ROOT, 'packages', 'com.vendor.product', 'meta', 'installscript.qs'),
        os.path.join(ROOT, 'packages', 'com.vendor.product', 'meta', 'package.xml'),
    ]
    
    jinja_templates = []
    for t in templates:
        e = jinja2.Environment()
        with open(t + '.template') as f:
            jinja_templates.append(e.from_string(f.read()))
    
    return jinja_templates, templates

def grab_version():
    ptn = re.compile(r'^#define\s+PROGRAM_VERSION\s+"([0-9\.]+)"\s*$')
    
    with open(os.path.join(ROOT, '..', 'src', 'config.h')) as f:
        for line in f.readlines():
            m = re.match(ptn, line)
            if m:
                return m.group(1)

if __name__ == '__main__':
    main()
