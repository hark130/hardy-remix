# I got tired of manually installing Radamsa (and now syslog) every time I utilize the
#	American Fuzzy Lop++ (AFL++) Docker image.  Time to inherit our own.
#
# USAGE:
# 	docker build devops/docker/HARE_AFL/ --tag hare_afl:latest
#	docker image list
#	docker run -ti --rm --mount type=tmpfs,destination=/ramdisk -e AFL_TMPDIR=/ramdisk -v `pwd`:/HARE hare_afl:latest


FROM aflplusplus/aflplusplus:latest

# SYSLOG
# 1. Copy in the crontab file
COPY etc_cron.d_syslog-crontab /etc/cron.d/syslog-crontab
# 2. Install crontab, install the necessary crontab table (see: crontab), and start cron at login
RUN apt update && apt install --yes rsyslog && chmod 0644 /etc/cron.d/syslog-crontab && crontab /etc/cron.d/syslog-crontab && echo "/usr/sbin/cron" >> /root/.bashrc

# RADAMSA
# Clone, build, and install Radamsa from source
RUN cd / && git clone https://gitlab.com/akihe/radamsa.git && cd radamsa && make && make install

# AFL++
# Compile AFL++ with memory sanitizer
RUN cd /AFLplusplus && make ASAN_BUILD=1
