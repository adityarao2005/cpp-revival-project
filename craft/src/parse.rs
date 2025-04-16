use crate::project;
use std::env;
use std::fs;
use std::io;

type Result<T> = std::result::Result<T, std::io::Error>;

mod init {
    type Result<T> = std::result::Result<T, std::io::Error>;
    use crate::project;
    use std::io;

    /// Reads the project name from the command line
    pub fn read_project_name() -> Result<String> {
        // First ask user the type of project they want to create
        println!("What is the name of the project?");
        let mut project_name = String::new();
        io::stdin().read_line(&mut project_name)?;
        project_name = project_name.trim().to_string();
        Ok(project_name)
    }

    pub fn read_project_type() -> Result<project::ProjectType> {
        println!(
            "Are you building a static library, shared library, or an executable? (sll/dll/exe)"
        );
        let mut project_type = String::new();
        io::stdin().read_line(&mut project_type)?;
        project_type = project_type.trim().to_string();
        let project_type = match project_type.as_str() {
            "sll" => project::ProjectType::StaticLibrary,
            "dll" => project::ProjectType::SharedLibrary,
            "exe" => project::ProjectType::Executable,
            _ => {
                println!("Invalid input. Please enter 'l' for library or 'e' for executable.");
                return Err(io::Error::new(io::ErrorKind::Other, "Invalid project type"));
            }
        };
        Ok(project_type)
    }

    pub fn read_cpp_version() -> Result<project::CPPVersion> {
        println!("What version of C++ are you using? (11/14/17/20/23)");
        let mut cpp_version = String::new();
        io::stdin().read_line(&mut cpp_version)?;
        cpp_version = cpp_version.trim().to_string();
        let cpp_version = match cpp_version.as_str() {
            "11" => project::CPPVersion::CPP11,
            "14" => project::CPPVersion::CPP14,
            "17" => project::CPPVersion::CPP17,
            "20" => project::CPPVersion::CPP20,
            "23" => project::CPPVersion::CPP23,
            _ => {
                println!("Invalid input. Please enter '11', '14', '17', '20', or '23'.");
                return Err(io::Error::new(io::ErrorKind::Other, "Invalid C++ version"));
            }
        };
        Ok(cpp_version)
    }
}

/// Precondition: Current Directory needs to be empty
/// Task: Create a new project file and all the directories underneath it for C++ project development
/// Postcondition: Creates the project file and all the directories underneath it
pub fn init() -> Result<()> {
    // Get the current path, and check if it's empty
    let current_path = env::current_dir()?;
    let read_dir = fs::read_dir(current_path)?;
    let empty_dir = read_dir.count() == 0;

    if !empty_dir {
        println!("Current directory is not empty. Please run this command in an empty directory.");
        return Err(io::Error::new(
            io::ErrorKind::Other,
            "Directory is not empty",
        ));
    }

    println!("Initializing a new project...");

    // First ask user the type of project they want to create
    let project_name = init::read_project_name()?;
    let project_type = init::read_project_type()?;
    let cpp_version = init::read_cpp_version()?;

    /*
    - (current directory)
    +----- src/ (created)
    +----- test/ (created)
    +----- include/ (created)
    +----- build/ (created)
    +----- lib/ (created)
    +----- project.json
    */
    fs::create_dir("./src")?;
    fs::write(
        "./src/main.cpp",
        "
#include <iostream>

int main() {
    std::cout << \"Hello, World!\" << std::endl;
    return 0;
}
    ",
    )?;
    fs::create_dir(project_name.clone())?;
    fs::create_dir(format!("./{}/test", project_name.clone()))?;
    fs::create_dir(format!("./{}/src", project_name.clone()))?;
    fs::create_dir(format!("./{}/include", project_name.clone()))?;
    fs::create_dir(format!("./{}/lib", project_name.clone()))?;

    // Create a new project file
    // TODO: complete the rest... series of prompts to get the project name, version, etc.
    let project_file = project::ProjectFile {
        dependencies: vec![],
        description: None,
        license: None,
        name: project_name,
        version: String::from("0.1.0"),
        project: project::Project {
            cpp_version,
            include_dirs: vec![String::from("/include")],
            include_files: vec![],
            src_dirs: vec![String::from("/src")],
            src_files: vec![],
            lib_dirs: vec![String::from("/lib")],
            test_dirs: vec![String::from("/test")],
            project_type,
            build: project::BuildConfig {
                build_dir: String::from("/build"),
                macros: vec![],
                warning_level: project::WarningLevel::W4,
            },
        },
    };

    project::write_project_file(&project_file)?;

    println!("Project file created successfully.");

    Ok(())
}

pub fn build() {
    // TODO: Implement the build function
    todo!();
}

pub fn run() {
    // TODO: Implement the run function
    todo!();
}

pub fn test() {
    // TODO: Implement the test function
    todo!();
}

pub fn export() {
    // TODO: Implement the export function
    todo!();
}

pub fn clean() {
    // TODO: Implement the clean function
    todo!();
}

pub fn install() {
    // TODO: Implement the install function
    todo!();
}
