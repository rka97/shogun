import sys
import os.path
import json
import re

class_hierarchy = {}


def get_include_path(h):
	# find *last* occurence of "shogun" dir in header
	src_dir = "src"
	tests_dir = "tests"
	tails = []
	head, tail = os.path.split(h)
	while tail != src_dir and tail != tests_dir and len(head) > 0:
		tails += [tail]
		head, tail = os.path.split(head)
	tails += [tail]

	# construct include path from collected tails
	tails.reverse()
	return os.path.join(*(tails))


def subclasses_of(class_name):
	if class_name not in class_hierarchy:
		return []
	class_info = class_hierarchy[class_name]
	subclasses_names = list(class_info["subclasses"])
	subclasses = []
	for subclass_name in subclasses_names:
		subclasses.append(class_hierarchy[subclass_name])
		subclasses += subclasses_of(subclass_name)
	return subclasses


def include_subclasses_of(class_name):
	lines = []
	subclasses = subclasses_of(class_name)
	for subclass in subclasses:
		lines.append("#include <{}>\n".format(subclass["include_path"]))
	return lines


def comma_separated_subclasses_of(class_name):
	line = ""
	subclasses = subclasses_of(class_name)
	for subclass in subclasses:
		line += "{},".format(subclass["class_name"])
	line = line[:-1]
	return [line]


if __name__ == '__main__':
	print(sys.argv)
	template_files = sys.argv[1:(len(sys.argv) - 1) // 2 + 1]
	output_files = sys.argv[(len(sys.argv) - 1) // 2 + 1:]
	class_hierarchy_path = "../../class_hierarchy.json"

	with open(class_hierarchy_path) as f:
		class_hierarchy = json.load(f)
		
	arg_matcher = re.compile(r"\((.*)\)")
	file_idx = 0
	for file_name in template_files:
		if file_name.find("external") > 0:
			continue
		try:
			lines = open(file_name).readlines()
		except:  # python3 workaround
			lines = open(file_name, encoding='utf-8', errors='ignore').readlines()

		output_lines = []
		line_nr = 0
		while line_nr < len(lines):
			line = lines[line_nr]
			idx1 = line.find("%subclasses_of")
			idx2 = line.find("%include_subclasses_of")
			end = line.find(")")
			if idx1 != -1:
				output_lines.append(line[:idx1])
				arg = arg_matcher.search(line).groups()[0]
				output_lines += comma_separated_subclasses_of(arg)
				if end != -1 and end < len(line) - 1:
					output_lines.append(line[end+1:])
			elif idx2 != -1:
				arg = arg_matcher.search(line).groups()[0]
				output_lines += include_subclasses_of(arg)
			else:
				output_lines.append(line)
			line_nr += 1

		with open(output_files[file_idx], 'w') as output_file:
			output_file.writelines("%s" % l for l in output_lines)
		file_idx += 1
