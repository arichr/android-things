#!/bin/bash

if [[ -n "$1" && -f "$1" ]]; then
	echo "- Found an existing selinux file."
	selinuxctx=$(<"$1")
elif [[ -z "$1" ]]; then
	echo "* No SELinux contexts were specified. Trying to download one from Google..."

	curl -h >/dev/null 2>/dev/null
	if [[ $? -ne 0 ]]; then
		echo "! curl is not installed. Cannot proceed."
		return 127	
	fi

	base64 --help >/dev/null 2>/dev/null
	if [[ $? -ne 0 ]]; then
		echo "! base64 is not installed. Cannot proceed."
		return 127	
	fi

	curl "https://android.googlesource.com/platform/system/sepolicy/+/master/private/file_contexts?format=TEXT" | base64 -d | tee /tmp/.selinux_context_genselinuxctx >/dev/null
	selinuxctx=$(</tmp/.selinux_context_genselinuxctx)
	rm /tmp/.selinux_context_genselinuxctx
else
	echo "Usage: $0 [selinux]"
	echo "    Generate a SELinux context for your files."
	echo -e "\nParameters:"
	echo "    selinux - If selinux is not present it will be"
	echo "              downloaded from the Google repository."
	echo -e "\nGitHub:"
	echo "    https://github.com/arichr/android-things"
	return
fi

echo "- Generating SELinux context..."
context_rules=$(echo "$selinuxctx" | grep -v "#")
for ctx_rule in "$context_rules"
do
	context="${ctx_rule##* }"
	regex="${ctx_rule% *}"
	files=$(find . -maxdepth 1 -regextype egrep -regex ".*$regex" -printf '/%P\n')
	for target_file in $files
	do
		echo "- Found: $target_file"
		echo "$target_file $context" >> file_contexts
	done 
done

echo "- Done!"
