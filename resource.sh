#!/bin/bash
gdk-pixbuf-csource --build-list \
    inline_broken   resource/broken.png   \
    inline_deferred resource/deferred.png \
    > resource.h
