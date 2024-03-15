# Introduction

This sample application is used as part of the interview process for Software Developers at Synertec.

Candidates are required to complete the assigned task within three and a half hours and submit to Synertec before moving onto the next interview phase.

# Submission

Candidates should upload their completed solution to their personal GitHub account and contact Synertec once complete.

The submission will then be reviewed as part of the interview process.

# The Application

The application is designed to perform maintenance on folders by moving files from one folder to another and deleting aged files.

It is written in C++ using MFC.

Currently a code change is required whenever new folders need to be monitored or existing folders need to have their age parameters amended.

# Set up

Candidates should download Visual Studio Community Edition if they do not already have it.

The application has a reliance on the following components within Visual Studio:

1) C++ MFC for latest v143 build tools (x86 & x64)

Candidates should copy 'TargetFolder' from the project folder to their C:\Temp\ directory. This folder contains the folder structure and files referred to within the code.

Time spent setting up is not considered part of the assessment.

# Task

Candidates are tasked with refactoring the application to achieve two objectives:

1) Minimise code duplication
2) Allow the application to accept a configuration file which controls all variable parts of the application

All current application functionality must be maintained during the refactoring. The task can be considered complete if no code changes are required to:

1) Monitor a new folder for file movement or deletion
2) Amend the retention days for file deletion
3) Exclude certain file types from deletion

An example configuration file has been provided to assist with getting the task started. It will need to be extended to achieve full coverage of the current application functionality. This can be found within the project, it is named 'Configuration.ini'

Candidates have three and a half hours to complete the task. There is no expectation to add unit tests or refactor the application beyond the two stated objectives. Additionally, candidates are permitted to use online resources to aid them in crafting a solution.
