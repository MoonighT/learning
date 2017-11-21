FROM ubuntu:16.04

WORKDIR /workspace
# Copy code.
COPY ./deploy /workspace
# Set volume
VOLUME /workspace/log
# Execute package scripts.
RUN ./deploy/install.sh 
ENTRYPOINT ["/bin/bash", "./deploy/run.sh", "/workspace"]
