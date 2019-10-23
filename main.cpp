#include <curl/curl.h>
#include <iostream>
#include <stdio.h>
#include <memory>
#include <stdint.h>
#include <chrono>
#include <math.h>

int64_t t_started = 0;

int64_t get_time() {
	return (int64_t)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}

size_t c_writedata(void* ptr, size_t sz, size_t nmemb, void* stream) {

	FILE* fstream = (FILE*)stream;
	size_t written = fwrite(ptr, sz, nmemb, fstream);

	// No error
	if (written > 0) {
		return written;
	}

	// Check if an error happened of the write reached EOF
	if (feof(fstream) != 0) {
		return written;
	}

	std::cout << "Error in writing download data in file: " <<
		ferror(fstream) << std::endl;

	return written;
}

int c_progress_callback(void* ptr, curl_off_t totalDownloadSize,
	curl_off_t finishedDownloadSize, curl_off_t totalToUpload,
	curl_off_t nowUploaded) {

	int64_t cur_epoch_ms = (int64_t)get_time();
	int64_t time_sec = (cur_epoch_ms - t_started) / 1000;
	time_sec = time_sec == 0 ? 1 : time_sec;
	int64_t bytes_per_second = finishedDownloadSize / time_sec;
	bytes_per_second = bytes_per_second == 0 ? 1 : bytes_per_second;
	int64_t bytes_left = totalDownloadSize - finishedDownloadSize;
	int64_t secs_left = bytes_left / bytes_per_second;

	std::cout << bytes_left << " bytes left, " << secs_left << " seconds left, at " << bytes_per_second / 1024 << " kB/s" << std::endl;

	return 0;
}

int main(int argc, char** argv) {

	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != 0) {
		std::cout << "curl_global_init failed with error: " << ret << std::endl;
		return false;
	}

	CURL* curl_handle = curl_easy_init();
	if (curl_handle == NULL) {
		std::cout << "Failed calling curl_easy_init" << std::endl;
		return false;
	}

	std::shared_ptr<void> defer(nullptr, [&](...){
		curl_easy_cleanup(curl_handle);
		curl_global_cleanup();
	});

	// Open a stream in which the curl data callback writes
	FILE* fh = fopen("./out", "w");
	if (fh == NULL) {
		std::cout << "Failed opening download dest file" << std::endl;
		return 1;
	}

	std::string url = "https://plantmonster.net/videos/lizerds.mp4";

	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());

	// Don't use curl's own progress meter
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);

	// Assign the main write callback
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, c_writedata);

	// custom pointer passed to write callback
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)fh);

	// assign custom progress callback
	curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION, c_progress_callback);

	// custom pointer passed to progress callback
	int foo = 5;
	curl_easy_setopt(curl_handle, CURLOPT_XFERINFODATA, (void*)&foo);

	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "foo/1.0");

	t_started = get_time();

	// start downloading
	CURLcode res = curl_easy_perform(curl_handle);

	fclose(fh);

	if (res != CURLE_OK) {
		std::cout << "Curl download failed with error: " << res <<
			": " << curl_easy_strerror(res) << std::endl;
		return 1;
	}

	std::cout << "Download finished" << std::endl;

	return 0;
}
