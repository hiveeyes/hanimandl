$(eval venvpath     := .venv)
$(eval python       := $(venvpath)/bin/python)
$(eval pip          := $(venvpath)/bin/pip)
$(eval platformio   := $(venvpath)/bin/platformio)


build: setup-virtualenv
	$(platformio) run

setup-virtualenv:
	@test -e $(python) || `command -v virtualenv` --python=python3 $(venvpath)
	$(pip) install platformio
