#!/bin/bash
gdk-pixbuf-csource --build-list inline_frame resource/frame.svg inline_broken resource/broken.svg > resource.h
