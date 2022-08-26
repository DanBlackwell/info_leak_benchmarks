# FROM ubuntu:20.04

FROM ubuntu:20.04 AS aflplusplus
#FROM ubuntu:22.04 AS aflplusplus
LABEL "maintainer"="afl++ team <afl@aflplus.plus>"
LABEL "about"="AFLplusplus docker image"

ARG DEBIAN_FRONTEND=noninteractive

env NO_ARCH_OPT 1

RUN apt-get update && \
    apt-get -y install --no-install-suggests --no-install-recommends \
    automake \
    ninja-build \
    bison flex \
    build-essential \
    git \
    python3 python3-dev python3-setuptools python-is-python3 \
    libtool libtool-bin \
    libglib2.0-dev \
    wget vim jupp nano bash-completion less \
    apt-utils apt-transport-https ca-certificates gnupg dialog \
    libpixman-1-dev \
    gnuplot-nox \
    gtk-doc-tools autopoint intltool libdbus-glib-1-dev \
    valgrind uuid-dev default-jre python3 zip \
    && rm -rf /var/lib/apt/lists/*

RUN echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-12 main" >> /etc/apt/sources.list && \
    wget -qO - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN echo "deb http://ppa.launchpad.net/ubuntu-toolchain-r/test/ubuntu focal main" >> /etc/apt/sources.list && \
    apt-key adv --recv-keys --keyserver keyserver.ubuntu.com 1E9377A2BA9EF27F

RUN apt-get update && apt-get full-upgrade -y && \
    apt-get -y install --no-install-suggests --no-install-recommends \
    gcc-10 g++-10 gcc-10-plugin-dev gcc-10-multilib gcc-multilib gdb lcov \
    clang-12 clang-tools-12 libc++1-12 libc++-12-dev \
    libc++abi1-12 libc++abi-12-dev libclang1-12 libclang-12-dev \
    libclang-common-12-dev libclang-cpp12 libclang-cpp12-dev liblld-12 \
    liblld-12-dev liblldb-12 liblldb-12-dev libllvm12 libomp-12-dev \
    libomp5-12 lld-12 lldb-12 llvm-12 llvm-12-dev llvm-12-runtime llvm-12-tools \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 0
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 0

ENV LLVM_CONFIG=llvm-config-12
ENV AFL_SKIP_CPUFREQ=1
ENV AFL_TRY_AFFINITY=1
ENV AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1

RUN useradd postgres

RUN apt-get update && \
    apt-get install -y make \
                       vim \
                       git \
                       subversion \
                       clang \
                       libtool \
                       wget \
                       parallel \
                       flex \
                       bison \
                       perl \
                       readline-common \
                       libreadline-dev \
                       zlib1g-dev


# RUN mkdir /usr/local/pgsql && \
RUN mkdir -p /usr/local/pgsql/data
RUN chown -R postgres /usr/local/pgsql
# CMD tail -f /dev/null

COPY ./Dockerfile /app/

COPY ./AFL_info_leakage /app/AFL_info_leakage
WORKDIR /app/AFL_info_leakage
RUN ls && make clean && make && make install

WORKDIR /app/

RUN wget https://www.antlr.org/download/antlr-4.8-complete.jar && \
    cp -f antlr-4.8-complete.jar /usr/local/lib

RUN pushd /app/AFL_info_leakage/custom_mutators/Grammar-Mutator && \
    make GRAMMAR_FILE=grammars/sql_grammar.json && \
    popd

COPY ./leakage_test /app/leakage_test

WORKDIR /app/leakage_test/NetworkManager_CVE-2011-1943
RUN ./build.sh

WORKDIR /app/leakage_test/atalk
RUN ./build.sh

WORKDIR /app/leakage_test/bind9_cve_2021_25216
RUN ./build.sh

WORKDIR /app/leakage_test/rds_kernel_CVE-2019-16714
RUN ./build.sh

WORKDIR /app/leakage_test/underflow
RUN ./build.sh

WORKDIR /app/leakage_test/postgres-cve-2021-3393
RUN ./build.sh

WORKDIR /app/leakage_test/OpenSSL_CVE-2014-0160
RUN ./build.sh

WORKDIR /app/leakage_test/IFSpec_banking/
RUN ./build.sh

WORKDIR /app/leakage_test/IFSpec_password_check/
RUN ./build.sh

WORKDIR /app/leakage_test/IFSpec_reviewers/
RUN ./build.sh


WORKDIR /app/
CMD ./begin_fuzzing 1200










# WORKDIR /app/leakage_test/postgres-cve-2021-3393
# RUN chown -R postgres /app
# 
# RUN CC=/app/AFL_info_leakage/afl-clang-fast CXX=/app/AFL_info_leakage/afl-clang-fast++ ./configure && make && make install
# 
# USER postgres
# 
# # RUN echo "PATH=$PATH:/usr/local/pgsql/bin/" >> ~/.bash_profile && \
# #  echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/pgsql/lib" >> ~/.bash_profile && \
# WORKDIR /app/leakage_test/postgres-cve-2021-3393
# RUN pwd; AFL_USE_ASAN=1 /app/AFL_info_leakage/afl-clang-fast postgres_leak.c ../json.c ../base64.c ../memory.c ../decode_inputs.c -I../ -I/usr/local/pgsql/include -L/usr/local/pgsql/lib -lpq -lm -Wall -o leaks
# 
# ENV PATH=$PATH:/usr/local/pgsql/bin/
# ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/pgsql/lib
# 
# CMD (initdb -D /usr/local/pgsql/data && \
#  (postgres -D /usr/local/pgsql/data > logfile 2>&1 &) && \
#  /app/AFL_info_leakage/afl-fuzz -i FUZZ_IN/ -o FUZZ_OUT/ -x /app/AFL_info_leakage/dictionaries/sql.dict -- ./leaks > fuzzing.log); \
#  tail -f /dev/null
