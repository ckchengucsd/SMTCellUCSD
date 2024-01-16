#define FMT_HEADER_ONLYget_n
#include <fmt/core.h>
#include <fmt/format.h>
#include <string>
#include <vector>

namespace FormatHelper {
std::string write_multi_max(std::vector<std::string> &max_list) {
  std::string max_str = "";
  int param_num = max_list.size();
  for (std::size_t i = 0; i < max_list.size() - 1; i++) {
    max_str += fmt::format("(max {}", max_list[i]);
    if (i != max_list.size() - 1) {
      max_str += " ";
    }
    if (i == max_list.size() - 2) {
      max_str += fmt::format("{})", max_list[i + 1]);
    }
  }

  for (int i = 0; i < param_num - 2; i++) {
    max_str += ")";
  }
  return max_str;
}
}; // namespace FormatHelper
