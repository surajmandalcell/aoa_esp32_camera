.PHONY: venv deps run

venv:
	python -m venv .venv

deps: venv
	.venv/Scripts/pip install -r requirements.txt

run: deps
	.venv/Scripts/python aoa_esp32_camera.py
