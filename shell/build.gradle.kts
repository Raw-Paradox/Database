plugins {
    java
}

group = "org.example"
version = "1.0-SNAPSHOT"

repositories {
    mavenCentral()
}

dependencies {
    testCompile("junit", "junit", "4.12")
    implementation("com.squareup.retrofit2:retrofit:2.7.1")
    implementation("com.squareup.retrofit2:converter-gson:2.5.0")
}
