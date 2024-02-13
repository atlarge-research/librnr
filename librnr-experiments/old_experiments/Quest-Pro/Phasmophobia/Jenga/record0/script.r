# Install and load required packages
if (!requireNamespace("tidyverse", quietly = TRUE)) {
  install.packages("tidyverse")
}
if (!requireNamespace("knitr", quietly = TRUE)) {
  install.packages("knitr")
}
if (!requireNamespace("forcats", quietly = TRUE)) {
  install.packages("forcats")
}
if (!requireNamespace("data.table", quietly = TRUE)) {
  install.packages("data.table")
}
if (!requireNamespace("cowplot", quietly = TRUE)) {
  install.packages("cowplot")
}
if (!requireNamespace("gghighlight", quietly = TRUE)) {
  install.packages("gghighlight")
}
if (!requireNamespace("zoo", quietly = TRUE)) {
  install.packages("zoo")
}
if (!requireNamespace("RColorBrewer", quietly = TRUE)) {
  install.packages("RColorBrewer")
}

library(tidyverse)
theme_set(theme_bw())
library(knitr)
library(forcats)
library(data.table)
library(cowplot)
library(gghighlight)
library(zoo)
library(RColorBrewer)

# Define the saveplot function
saveplot <- function(filename, ...) {
  ggsave(filename, ...)
  knitr::plot_crop(filename)
}

# Read FPS data from log file
fps <- system('grep -Po "(?<=FPS=)[0-9]+" logcat_VrApi.log', intern = TRUE)

# Create a tibble with FPS data
data <- tibble(fps) %>%
  mutate(fps = as.numeric(fps)) %>%
  mutate(ts = 0:(n() - 1)) %>%
  select(ts, everything())

# Plot the data using ggplot2
data %>%
  ggplot(aes(x = ts, y = fps)) +
  geom_line() +
  ylim(0, NA) +
  theme_half_open() +
  background_grid()


if (file.exists("logcat_VrApi.log")) {
cpu_freq <- system('grep -Po "(?<=CPU4/GPU=)[0-9]/[0-9],[0-9]+/[0-9]+(?=MHz,OC)" logcat_VrApi.log | cut -d, -f 2 | cut -d/ -f 1', intern = TRUE)
data <- tibble(cpu_freq) %>% mutate(cpu_freq = as.numeric(cpu_freq)) %>% mutate(ts = 0:(n()-1)) %>% select(ts, everything())
data %>%
ggplot(aes(x = ts, y=cpu_freq)) +
geom_line() +
ylim(0, NA) +
labs(y = "MHz", x = "time [s]") +
theme_half_open() + background_grid()
}

# Uncomment and use the following line to save the plot
# saveplot("your_filename.png")


