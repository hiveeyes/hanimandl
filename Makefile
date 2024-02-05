$(eval venvpath     := .venv)
$(eval python       := $(venvpath)/bin/python)
$(eval pip          := $(venvpath)/bin/pip)
$(eval platformio   := $(venvpath)/bin/platformio)
$(eval sphinx-autobuild := $(venvpath)/bin/sphinx-autobuild)

setup-virtualenv:
	@test -e $(python) || python3 -m venv $(venvpath)
	$(pip) install platformio

.PHONY: build
build: setup-virtualenv
	$(platformio) run

.PHONY: docs
docs: setup-virtualenv
	$(pip) install -r docs/requirements.txt -r docs/requirements-dev.txt
	$(sphinx-autobuild) docs docs/_build/ --open-browser
