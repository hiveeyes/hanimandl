# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'HaniMandl'
copyright = '2018-2024, The HaniMandl Developers'
author = 'The HaniMandl Developers'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "myst_parser",
    "sphinx_copybutton",
    "sphinx.ext.intersphinx",
]

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = True

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_static_path = ['_static']
html_title = "HaniMandl"


# -- Options for Sphinx-copybutton -------------------------------------------
copybutton_remove_prompts = True
copybutton_line_continuation_character = "\\"
copybutton_prompt_text = r">>> |\.\.\. |\$ |sh\$ |PS> |cr> |mysql> |In \[\d*\]: | {2,5}\.\.\.: | {5,8}: "
copybutton_prompt_is_regexp = True


# -- Options for MyST --------------------------------------------------------
myst_heading_anchors = 3
myst_enable_extensions = [
    "attrs_block",
    "attrs_inline",
    "colon_fence",
    "deflist",
    "fieldlist",
    "html_admonition",
    "html_image",
    "linkify",
    "replacements",
    "strikethrough",
    "substitution",
    "tasklist",
]
