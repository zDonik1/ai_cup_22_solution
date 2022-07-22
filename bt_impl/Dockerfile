FROM gcc:12
RUN apt-get update && apt-get install -y jq cmake unzip wget libzmq3-dev libboost-dev
RUN mkdir /bt_build && cd /bt_build \
    && wget https://github.com/BehaviorTree/BehaviorTree.CPP/archive/refs/tags/3.7.0.tar.gz \
    && tar xvf 3.7.0.tar.gz \
    && cmake -B build BehaviorTree.CPP-3.7.0 \
    && cmake --build build --parallel \
    && cmake --install build \
    && cd / && rm -rf bt_build

ENV MOUNT_POINT="/opt/mount-point"
ENV SOLUTION_CODE_PATH="/opt/client/solution"
COPY . $SOLUTION_CODE_PATH
WORKDIR $SOLUTION_CODE_PATH
CMD ["bash", "entrypoint.sh"]
