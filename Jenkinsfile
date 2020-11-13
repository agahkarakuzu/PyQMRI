pipeline { 
  agent {     
    dockerfile {
      filename 'Dockerfile'
      args '--gpus all -u root'
      }
  }
  stages {
    stage('Build') {
      steps {
        sh 'pip3 install -r requirements.txt'
        sh 'pip3 install -e .'
      }
    }
    stage('Pylint') {
      steps {
        sh 'pylint -ry --output-format=parseable --exit-zero ./pyqmri > pylint.log'
      }
    }
    stage('Unittests') {
      steps {
        sh 'pytest --junitxml results_unittests_LinOp.xml --cov=pyqmri test/unittests/test_LinearDataOperator.py'
        sh 'coverage xml -o coverage_unittest_LinOp.xml'
        sh 'pytest --junitxml results_unittests_grad.xml --cov=pyqmri test/unittests/test_gradient.py'
        sh 'coverage xml -o coverage_unittest_grad.xml'
        sh 'pytest --junitxml results_unittests_symgrad.xml --cov=pyqmri test/unittests/test_symmetrized_gradient.py'
        sh 'coverage xml -o coverage_unittest_symgrad.xml'
      }
    }
    stage('Integrationtests') {
      steps {
        sh 'ipcluster start&'
        sh 'pytest --junitxml results_integrationtests_single_slice.xml --cov=pyqmri --integration-cover test/integrationtests/test_integration_test_single_slice.py'
        sh 'coverage xml -o coverage_integrationtest_single_slice.xml'
        sh 'pytest --junitxml results_integrationtests_multi_slice.xml --cov=pyqmri --integration-cover test/integrationtests/test_integration_test_multi_slice.py'
        sh 'coverage xml -o coverage_integrationtest_multi_slice.xml'
        sh 'ipcluster stop&'
      }
    }
  }
  post {
      always {
          sh '../../testquality-linux results_unittests_LinOp.xml --project_name=PyQMRI --plan_name="LinearOperator"'
          sh '../../testquality-linux results_unittests_grad.xml --project_name=PyQMRI --plan_name="Gradient"'
          sh '../../testquality-linux results_unittests_symgrad.xml --project_name=PyQMRI --plan_name="Symmetrizded Gradient"'
          sh '../../testquality-linux results_integrationtests_single_slice.xml --project_name=PyQMRI --plan_name="Single Slice Reconstruction"'
          sh '../../testquality-linux results_integrationtests_multi_slice.xml --project_name=PyQMRI --plan_name="Multi Slice Reconstruction"'
          cobertura coberturaReportFile: 'coverage_unittest_LinOp.xml, coverage_unittest_grad.xml, coverage_unittest_symgrad.xml, coverage_integrationtest_single_slice.xml, coverage_integrationtest_multi_slice.xml', enableNewApi: true
          junit 'results*.xml'
          recordIssues enabledForFailure: true, tool: pyLint(pattern: 'pylint.log')
          cleanWs()
      }
  }
}
