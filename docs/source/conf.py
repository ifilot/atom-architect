# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Atom Architect'
copyright = '2026, Ivo Filot'
author = 'Ivo Filot'
release = '1.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx_copybutton",
    "sphinx_tabs.tabs",
]

html_theme = "sphinx_rtd_theme"
html_title = "Atom Architect User Manual"
html_logo = "_static/img/atom_architect_256.png"
exclude_patterns = []
templates_path = ['_templates']
exclude_patterns = []
html_static_path = ['_static']
