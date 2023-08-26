import os

p_list = [5, 6, 7, 8]
n_iter = 50
def run_test(method):
	t_list = {}
	for p in p_list:
		os.system(f"build/test_sort {10 ** p} {method} {n_iter} > build/output.txt")
		with open("build/output.txt") as f:
			for line in f.readlines():
				pass
			t_list[p] = float(line)
	return t_list

t_serial = run_test(0)
print(t_serial)
t_parasort = run_test(1)
print(t_parasort)
t_stdsort = run_test(2)
print(t_stdsort)

table = '| n | std::sort (serial) | std::sort (parallel) | std::parasort |\n'
table += '| --- | --- | --- | --- |\n'
for p in p_list:
	table += f"| $10^{p}$ | {t_serial[p]:.1f} | {t_stdsort[p]:.1f} ({t_serial[p] / t_stdsort[p]:.3f})| {t_parasort[p]:.1f} ({t_serial[p] / t_parasort[p]:.2f}) |\n"
with open('sort_timing.md', 'w') as f:
	f.write(table)
