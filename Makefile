$(eval venvpath  := .venv3)
$(eval python    := $(venvpath)/bin/python)
$(eval pip       := $(venvpath)/bin/pip)
$(eval pio       := $(venvpath)/bin/pio)


build: setup-virtualenv
	$(pio) run

setup-virtualenv:
	@test -e $(python) || `command -v virtualenv` --python=python3 $(venvpath)
	$(pip) install platformio
